/*	Copyright (c) 2001-2017, Institut für Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, H. Fechner, St. Vogelsang, A. Paepcke, J. Grunewald
	All rights reserved.

	This file is part of the IBK Library.

	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
	   list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation
	   and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors
	   may be used to endorse or promote products derived from this software without
	   specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
	ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


	This library contains derivative work based on other open-source libraries.
	See OTHER_LICENCES and source code headers for details.

*/

#include "IBK_configuration.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <algorithm>
#include <string>
#include <fstream>
#include <locale>
#include <cctype>
#include <cmath>

#ifdef _WIN32
#ifdef NOMINMAX
#include <windows.h>
#else
#define NOMINMAX
#include <windows.h>
#undef NOMINMAX
#endif
#endif

#include "IBK_StringUtils.h"
#include "IBK_Unit.h"
#include "IBK_UnitList.h"
#include "IBK_messages.h"
#include "IBK_Exception.h"
#include "IBK_assert.h"
#include "utf8/utf8.h"


#ifdef _WIN32

  #ifndef _WIN64

	#define IBK_USE_STOD

  #else

	#include "fast_float/fast_float.h"

  #endif

#else

  #include "fast_float/fast_float.h"

#endif


namespace IBK {


template <>
double string2val<double>(const std::string& str) {
	double val;
	if (str=="1.#QNAN")
		return std::numeric_limits<double>::quiet_NaN();
#ifdef IBK_USE_STOD
	// for 32-bit, use std::stod()
	size_t pos;
	try {
		val = std::stod(str, &pos); // may throw std::out_of_range or std::invalid_argument
		if (str.find_first_not_of(" \t\n", pos) != std::string::npos)
			throw std::exception();
	}
	catch (...) {
		throw IBK::Exception(IBK::FormatString("Could not convert '%1' into value.").arg(str), "[IBK::string2val<double>]");
	}
#else
	auto answer = fast_float::from_chars(str.data(), str.data()+str.size(), val);
	if (answer.ec != std::errc())
		throw IBK::Exception(IBK::FormatString("Could not convert '%1' into value.").arg(str), "[IBK::string2val<double>]");
#endif
	return val;
}

template <>
bool string2val<bool>(const std::string & str) {
	// only allow 'true' and 'false'
	if (str == "true" || str == "1")
		return true;
	else if (str == "false" || str == "0")
		return false;
	throw IBK::Exception(IBK::FormatString("Could not convert '%1' into bool.").arg(str), "[IBK::string2val<bool>]");
}


template <>
double string2valDef<double>(const std::string& str, const double & def) {
	if (str=="1.#QNAN")
		return std::numeric_limits<double>::quiet_NaN();
	double val;
#ifdef IBK_USE_STOD
	// for 32-bit, use std::stod()
	size_t pos;
	if (std::locale().name() != "C")
		setlocale(LC_ALL, "C");
	try {
		val = std::stod(str, &pos); // may throw std::out_of_range or std::invalid_argument
		if (str.find_first_not_of(" \t\n", pos) != std::string::npos)
			throw std::exception();
	}
	catch (...) {
		throw IBK::Exception(IBK::FormatString("Could not convert '%1' into value.").arg(str), "[IBK::string2val<double>]");
	}
#else
	auto answer = fast_float::from_chars(str.data(), str.data()+str.size(), val);
	if (answer.ec != std::errc())
		return def;
#endif
	return val;
}


void string2valueVector(const std::string & origStr, std::vector<double> & vec) {
	FUNCID(IBK::string2valueVector);
	// algorithm is simple - search for white-space delimiters and replace the first white-space after each non-white space
	// substr with \0 and remember its position, then parse all the substrings

	vec.clear();
	size_t strsize = origStr.size();
	if (strsize == 0)
		return; // empty string - no data in vector

	std::string str(origStr);
	size_t pos = 0;
	size_t numberStart = 0;

	bool inNumber = false;

	// process entire string, search until we toggle from "whitespace" to "number" and remember starting position of number
	// then continue until we toggle from "number" to "whitespace" and try to convert the string in-between
	while (pos < strsize) {
		char ch = str[pos];
		// do we have a whitespace char?
		if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
			// toggle to "whitespace mode"?
			if (inNumber) {
				inNumber = false;
				// change character to \0
				str[pos] = 0;
#ifdef IBK_USE_STOD
				// try to parse from begin of number to this position
				size_t charCount;
				try {
					double val = std::stod(str.data()+numberStart, &charCount);
					if (charCount != pos - numberStart)
						throw std::exception();
					vec.push_back(val);
				} catch (...) {
					throw IBK::Exception(IBK::FormatString("'%1' at character pos #2 is not a valid number.").arg(str.data()+numberStart).arg(numberStart), FUNC_ID);
				}
#else
				// try to parse from begin of number to this position
				double val;
				auto answer = fast_float::from_chars(str.data()+numberStart, nullptr, val);
				if (answer.ec != std::errc())
					throw IBK::Exception(IBK::FormatString("'%1' at character pos #2 is not a valid number.").arg(str.data()+numberStart).arg(numberStart), FUNC_ID);
				vec.push_back(val);
#endif // IBK_USE_STOD
			}
		}
		else {
			// toggle to "number" mode?
			if (!inNumber) {
				numberStart = pos;
				inNumber = true;
			}
		}
		++pos;
	}
	// if we are still in number mode at end of string, then try to parse the last number
	if (inNumber) {
#ifdef IBK_USE_STOD
		size_t charCount;
		try {
			double val = std::stod(str.data()+numberStart, &charCount);
			if (charCount != pos - numberStart)
				throw;
			vec.push_back(val);
		} catch (...) {
			throw IBK::Exception(IBK::FormatString("'%1' at character pos #2 is not a valid number.").arg(str.data()+numberStart).arg(numberStart), FUNC_ID);
		}
#else
		double val;
		auto answer = fast_float::from_chars(str.data()+numberStart, nullptr, val);
		if (answer.ec != std::errc())
			throw IBK::Exception(IBK::FormatString("'%1' at character pos #2 is not a valid number.").arg(str.data()+numberStart).arg(numberStart), FUNC_ID);
		vec.push_back(val);
#endif // IBK_USE_STOD
	}
}
// ---------------------------------------------------------------------------


void trim(std::string& str, const std::string& trimchars) {
	size_t f = str.find_first_not_of(trimchars);
	size_t l = str.find_last_not_of(trimchars);
	if (f!=std::string::npos && l!=std::string::npos)
		str = str.substr(f,l-f+1);
	else
		str.clear();
}
// ---------------------------------------------------------------------------

std::string trim_copy(const std::string& str, const std::string& trimchars) {
	size_t f = str.find_first_not_of(trimchars);
	size_t l = str.find_last_not_of(trimchars);
	if (f!=std::string::npos && l!=std::string::npos)
		return str.substr(f,l-f+1);
	return "";
}
// ---------------------------------------------------------------------------

std::string& trim_keyword(std::string& keyword) {
	if (keyword.empty())
		return keyword;
	size_t first = 0;
	size_t last = keyword.size()-1;
	if (keyword.at(0) == '[')  ++first;
	if (keyword.at(last) == ']')  --last;
	if (keyword.at(last) == '=')  --last;
	keyword = keyword.substr(first, last-first+1);
	return keyword;
}

/// Removes comments from the str (removes everything after and including the ; )
std::string& remove_comment(std::string& str) {
	std::size_t pos = str.find(";");
	if ((pos!=std::string::npos) && (pos<str.length()))
		str.erase(pos);
	return str;
}
// ---------------------------------------------------------------------------

/// Compares the strings case in-sensitve.
bool string_nocase_compare(const std::string& lhs, const std::string& rhs) {
	if (lhs.size()!=rhs.size()) return false;
	const char* s1=lhs.data();
	const char* s2=rhs.data();
	size_t n=lhs.size()+1;
#if defined(_MSC_VER)
	while (::tolower(*s1++)==::tolower(*s2++) && --n);
#else
	while (std::tolower(*s1++)==std::tolower(*s2++) && --n) {}
#endif
	return n==0;
}
// ---------------------------------------------------------------------------

bool string_nocase_less(const std::string& lhs, const std::string& rhs) {
	const char* s1=lhs.data();
	const char* s2=rhs.data();
	size_t n=std::min(lhs.size(), rhs.size())+1;
#if defined(_MSC_VER)
	while (::tolower(*s1++)==::tolower(*s2++) && --n);
#else
	while (std::tolower(*s1++)==std::tolower(*s2++) && --n) {}
#endif
	if (n==0)
		return rhs.size() > lhs.size();
	else
#if defined(_MSC_VER)
		return ::tolower(*(--s1)) < ::tolower(*(--s2));
#else
		return std::tolower(*(--s1)) < std::tolower(*(--s2));
#endif
}
// ---------------------------------------------------------------------------

/// Finds the string 'substr' in the string 'str' (case insensitive).
/// Returns either the position of the substr or string::npos.
std::size_t string_nocase_find(const std::string& str, const std::string& substr) {
	if (str.empty())    return std::string::npos;
	if (substr.empty()) return 0;
	const char* s1=str.data();
	const char* s2=substr.data();
	size_t size1=str.size();
	size_t pos = 0;
	while (pos<size1) {
#if defined(_MSC_VER)
		if (::tolower(*s1)==::tolower(*s2)) {
#else
		if (std::tolower(*s1)==std::tolower(*s2)) {
#endif
			size_t size2=substr.size();
			const char* s3=s1;
#if defined(_MSC_VER)
			while (--size2 && ::tolower(*(++s3))==::tolower(*(++s2))) {}
#else
			while (--size2 && std::tolower(*(++s3))==std::tolower(*(++s2))) {}
#endif
			if (!size2) return pos;
			s2 = substr.data(); // prepare for next trial
		}
		++s1;
		++pos;
	}
	if (pos==size1) return std::string::npos;
	else            return pos;
}
// ---------------------------------------------------------------------------

/// Extracts keyword and value from string "keyword = value"
bool extract_keyword(const std::string& line, std::string& keyword, std::string& value) {
	size_t pos = line.find('=');
	if (pos==std::string::npos) {
		value = line;
		keyword.clear();
		return false;
	}
	keyword = line.substr(0, pos);
	trim(keyword);
	value = line.substr(pos+1);
	trim(value);
	return true;
}
// ---------------------------------------------------------------------------

unsigned int extractFromParenthesis(const std::string & src, unsigned int defaultValue) {
	unsigned int val = 0; // just to make compiler happy
	std::string str;
	if (extractFromParenthesis(src, str, val) == ERT_Success)
		return val;
	else
		return defaultValue;
}
// ---------------------------------------------------------------------------

std::pair<unsigned int, double> extractFromParenthesis(const std::string & src,
													std::pair<unsigned int, double> defaultValue) {
	std::string::size_type pos = src.find("(");
	std::string::size_type pos2 = src.find(")");
	if (pos != std::string::npos && pos2 != std::string::npos && pos2 > pos) {
		std::string substr = src.substr(pos+1, pos2-pos-1);
		std::list<std::string> tokens;
		IBK::explode(substr,tokens,',');
		if(tokens.size() == 1)
		{
			try {
#if defined(_MSC_VER) && _MSC_VER >= 1700 //Visual Studio 2012
				defaultValue = std::make_pair
				(IBK::string2val<unsigned int>(tokens.front()), defaultValue.second);
#else
				defaultValue = std::make_pair<unsigned int, double>
				(IBK::string2val<unsigned int>(tokens.front()), (double)defaultValue.second);
#endif

			}
			catch (...) {}
		}
		else if(tokens.size() == 2)
		{
			try {
#if defined(_MSC_VER) && _MSC_VER >= 1700 //Visual Studio 2012
				defaultValue = std::make_pair
				(IBK::string2val<unsigned int>(tokens.front()),
				IBK::string2val<double>(tokens.back()) );
#else
				defaultValue = std::make_pair<unsigned int, double>
				(IBK::string2val<unsigned int>(tokens.front()),
				IBK::string2val<double>(tokens.back()) );
#endif

			}
			catch (...) {}
		}
	}
	return defaultValue;
}
// ---------------------------------------------------------------------------

std::vector<std::string> explode(const std::string & str, char delim, int maxTokens) {
	std::vector<std::string> tokens;
	std::string tmp;
	for (std::string::const_iterator it=str.begin(); it!=str.end(); ++it) {
		if (*it!=delim)
			tmp+=*it;
		else {
			if (tmp.empty()) continue;
			tokens.push_back(tmp);
			if (maxTokens != -1 && tokens.size() >= (size_t)maxTokens)
				return tokens;
			tmp.clear();
		}
	}
	if (tmp.size())
		tokens.push_back(tmp);
	return tokens;
}


size_t explode(const std::string& str, std::vector<std::string>& tokens, const std::string& delims, int explodeFlags) {
	tokens.clear();
	std::string tmp;

	for (std::string::const_iterator it=str.begin(); it!=str.end(); ++it) {
		bool delim_found = false;
		for (std::string::const_iterator tit = delims.begin(); tit != delims.end(); ++tit) {
			if (*it==*tit) {
				if (explodeFlags & EF_TrimTokens)
					IBK::trim(tmp, " \t\r\n");
				if (!tmp.empty() || (explodeFlags & EF_KeepEmptyTokens))
					tokens.push_back(tmp);
				tmp.clear();
				delim_found = true;
				break;
			}
		}
		if (!delim_found)
			tmp += *it;
	}
	if (!tmp.empty() || (explodeFlags & EF_KeepEmptyTokens)) {
		if (explodeFlags & EF_TrimTokens)
			IBK::trim(tmp, " \t\r\n"); // may cause tmp to become empty
		if (!tmp.empty() || (explodeFlags & EF_KeepEmptyTokens))
			tokens.push_back(tmp);
	}
	return tokens.size();
}
// ---------------------------------------------------------------------------


size_t explode(const std::string& str, std::list<std::string>& tokens, char delim, bool trimTokens) {
	std::string tmp;
	tokens.clear();
	for (std::string::const_iterator it=str.begin(); it!=str.end(); ++it) {
		if (*it!=delim)
			tmp+=*it;
		else {
			if (tmp.empty()) continue;
			if (trimTokens)
				IBK::trim(tmp);
			tokens.push_back(tmp);
			tmp.clear();
		}
	}
	if (tmp.size())
		tokens.push_back(tmp);
	// if 'str' was empty, return also a vector with exactly one empty string
	if (tokens.empty())
		tokens.push_back("");
	return tokens.size();
}
// ---------------------------------------------------------------------------

size_t explode(const std::string& str, std::vector<std::string>& tokens, char delim, bool trimTokens) {
	std::string tmp;
	tokens.clear();
	for (std::string::const_iterator it=str.begin(); it!=str.end(); ++it) {
		if (*it!=delim)
			tmp+=*it;
		else {
			if (tmp.empty()) continue;
			if (trimTokens)
				IBK::trim(tmp);
			tokens.push_back(tmp);
			tmp.clear();
		}
	}
	if (!tmp.empty()) {
		if (trimTokens)
			IBK::trim(tmp);
		tokens.push_back(tmp);
	}
	// if 'str' was empty, return also a vector with exactly one empty string
	if (tokens.empty())
		tokens.push_back("");
	return tokens.size();
}
// ---------------------------------------------------------------------------

size_t explode_in2(const std::string& str, std::vector<std::string>& tokens, char delim) {
	std::string tmp;
	tokens.clear();
	std::string::const_iterator it=str.begin();
	while( it!= str.end() && *it!=delim ) {
		tmp += *it;
		++it;
	}
	tokens.push_back(tmp);
	if( it != str.end()) {
		std::string tstr(it+1, str.end());
		tokens.push_back(tstr);
	}
	return tokens.size();
}
// ---------------------------------------------------------------------------

size_t explode_in2(const std::string& str, std::vector<std::string>& tokens, const std::string & delims) {
	std::string tmp;
	tokens.clear();
	std::string::const_iterator it=str.begin();
	while( it != str.end()) {
		std::string::const_iterator delit = delims.begin();
		for (; delit != delims.end(); ++delit) {
			if (*it == *delit)
				break;
		}
		if (delit != delims.end())
			break;
		tmp += *it;
		++it;
	}
	tokens.push_back(tmp);
	if( it != str.end()) {
		std::string tstr(it+1, str.end());
		tokens.push_back(tstr);
	}
	return tokens.size();
}
// ---------------------------------------------------------------------------

size_t explode(const std::string& str, std::list<std::string>& tokens,
	std::string& delims, const std::string& delimiter)
{
	std::string tmp;
	tokens.clear();
	delims.clear();
	int delim_count = 0;
	delims.resize(500); // 500 delimiters should be enough, right?
	for (std::string::const_iterator it=str.begin(); it!=str.end() && delim_count < 500; ++it) {
		bool delim_found = false;
		for (std::string::const_iterator tit = delimiter.begin(); tit != delimiter.end(); ++tit) {
			if (*it==*tit) {
				tokens.push_back(tmp);
				delims[delim_count++] = *tit;
				tmp.clear();
				delim_found = true;
				break;
			}
		}
		if (!delim_found) tmp += *it;
	}
	if (!tmp.empty()) {
		tokens.push_back(tmp);
		delims[delim_count] = ' ';
	}
	// if 'str' was empty, return also a vector with exactly one empty string
	if (tokens.empty())
		tokens.push_back("");
	return tokens.size();
}
// ---------------------------------------------------------------------------

void explode_csv(const std::string& str, std::list<std::string>& tokens) {
	std::string tmp;
	tokens.clear();
	bool in_quotes = false;
	for (std::string::const_iterator it=str.begin(); it!=str.end(); ++it) {
		if (*it == '\"')  {
			in_quotes = !in_quotes;
			tmp += *it;
		}
		else if (!in_quotes && *it == ',') {
			tokens.push_back(tmp);
			tmp.clear();
		}
		else {
			tmp+=*it;
		}
	}
	if (tmp.size())
		tokens.push_back(tmp);
}
// ---------------------------------------------------------------------------

void explode_csv(const std::string& str, std::vector<std::string>& tokens) {
	std::string tmp;
	tokens.clear();
	bool in_quotes = false;
	for (std::string::const_iterator it=str.begin(); it!=str.end(); ++it) {
		if (*it == '\"')  {
			in_quotes = !in_quotes;
			tmp += *it;
		}
		else if (!in_quotes && *it == ',') {
			tokens.push_back(tmp);
			tmp.clear();
		}
		else {
			tmp+=*it;
		}
	}
	if (tmp.size())
		tokens.push_back(tmp);
}
// ---------------------------------------------------------------------------

void explode_sections(std::istream& in,
					 const std::vector<std::string>& section_titles,
					 std::vector<std::string>& section_data,
					 const std::string & commentChars)
{

	size_t current_section = section_titles.size();
	section_data.clear();
	section_data.resize(section_titles.size());
	std::string line, keyword;
	while (std::getline(in, line)) {

		// skip empty lines
		std::string::size_type pos = line.find_first_not_of(" \t\r");
		if (pos == std::string::npos)
			continue;

		// skip comments, if first character in line is in commentChars,
		// this must be a comment line
		if (commentChars.find(line[pos]) != std::string::npos)
			continue;

		// try to extract the keyword by assuming the line holds a keyword, then
		// removing the []
		keyword = line;
		trim(keyword);
		trim_keyword(keyword);

		// is this a known keyword?
		std::vector<std::string>::const_iterator it = find(section_titles.begin(), section_titles.end(), keyword);

		// check for new keyword
		if (it != section_titles.end()) {
			current_section = (unsigned long)std::distance(section_titles.begin(), it);
		}
		else {

			// if no list keyword has been found yet, throw exception
			if (current_section==section_titles.size())
				throw IBK::Exception( IBK::FormatString("Expected section header in first non-empty line:\n'%1'").arg(line),
					"[IBK::explode_sections]");
			// add string line to current part
			section_data[current_section] += line + '\n';
		}
	}
}
// ---------------------------------------------------------------------------

void explode_sections(const std::vector<std::string>& data,
					 const std::vector<std::string>& section_titles,
					 std::vector<std::vector<std::string> >& section_data,
					 const std::string & commentChars)
{

	size_t current_section = section_titles.size();
	section_data.clear();
	section_data.resize(section_titles.size());
	std::string keyword;
	for(unsigned int i=0, size=(unsigned int)data.size(); i<size; ++i) {

		// skip empty lines
		std::string::size_type pos = data[i].find_first_not_of(" \t\r");
		if (pos == std::string::npos)
			continue;

		// skip comments, if first character in line is in commentChars,
		// this must be a comment line
		if (commentChars.find(data[i][pos]) != std::string::npos)
			continue;

		// try to extract the keyword by assuming the line holds a keyword, then
		// removing the []
		keyword = data[i];
		trim(keyword);
		trim_keyword(keyword);

		// is this a known keyword?
		std::vector<std::string>::const_iterator it = find(section_titles.begin(), section_titles.end(), keyword);

		// check for new keyword
		if (it != section_titles.end()) {
			current_section = distance(section_titles.begin(), it);
		}
		else {

			// if no list keyword has been found yet, throw exception
			if (current_section==section_titles.size())
				throw IBK::Exception( IBK::FormatString("Expected section header in first non-empty line:\n'%1'")
									  .arg(data[i]), "[IBK::explode_sections]");
			// add string line to current part
			section_data[current_section].push_back(data[i]);
		}
	}
}
// ---------------------------------------------------------------------------


void explode_sections(const std::string& str, const std::vector<std::string>& section_titles,
	std::vector<std::string>& section_data)
{
	std::stringstream strm(str);
	explode_sections(strm, section_titles, section_data);
}
// ---------------------------------------------------------------------------


void explode_section(const std::string& str, const std::string& section_title,
	std::vector<std::string>& section_data)
{
	std::stringstream in(str);
	std::string line, keyword;
	section_data.clear();
	while (getline(in, line)) {
		trim(remove_comment(line));
		keyword = line;
		if (line.empty()) continue;
		trim_keyword(keyword);
		if (keyword==section_title)
			section_data.push_back(""); // add a new section
		else {
			if (section_data.empty())
				throw IBK::Exception("Expected keyword/section title!","[explode_section]");
			// add string line to current section data
			section_data.back() += line + '\n';
		}
	}
}
// ---------------------------------------------------------------------------


void explode_section(const std::vector<std::string>& data, const std::string& section_title,
	std::vector<std::vector<std::string> >& section_data)
{
	int current_section = -1;
	section_data.clear();
	std::string keyword;
	for(unsigned int i=0, size=(unsigned int)data.size(); i<size; ++i) {

		// skip empty lines
		std::string::size_type pos = data[i].find_first_not_of(" \t\r");
		if (pos == std::string::npos)
			continue;

		// try to extract the keyword by assuming the line holds a keyword, then
		// removing the []
		keyword = data[i];
		trim(keyword);
		trim_keyword(keyword);

		// is this our section keyword?
		if (keyword == section_title) {
			section_data.push_back(std::vector<std::string>());
			current_section = (int)section_data.size()-1;
			continue;
		}

		// if no list keyword has been found yet, throw exception
		if (current_section==-1)
			throw IBK::Exception( IBK::FormatString("Expected section header in first non-empty line:\n'%1'")
								  .arg(data[i]), "[IBK::explode_section]");
		// add string line to current part
		section_data[current_section].push_back(data[i]);
	}
}
// ---------------------------------------------------------------------------


std::string format_double(double val, int precision, std::ios_base::fmtflags fmt, int width) {
	std::stringstream strm;
	strm.setf(fmt);
	strm << std::setprecision(precision);
	if (width != 0)
		strm.width(width);
	strm << val;
	return strm.str();
}
// ---------------------------------------------------------------------------


void break_string(const std::string& msg, std::vector<std::string>& lines,
	int line_width)
{
	// first tokenize the string
	std::list<std::string> tokens;
	std::string delims;
	explode(msg, tokens, delims, " \n");
	std::string current_line;
	lines.clear();
	std::string::const_iterator delit = delims.begin();
	for (std::list<std::string>::iterator it = tokens.begin(); it != tokens.end() && delit != delims.end(); ++it, ++delit) {
		if (current_line.size() + it->size() + 1 > static_cast<unsigned int>(line_width)) {
			lines.push_back( current_line );
			current_line.clear();
		}
		current_line += *it;
		if (*delit == '\n') {
			lines.push_back( current_line );
			current_line.clear();
		}
		else
			current_line += ' ';
	}
	lines.push_back(current_line); // if current_line is empty, msg was appended with a \n
}
// ---------------------------------------------------------------------------

/// Utility function to parse error/progress/debug message strings
void parse_msg_string(const std::string& msg, std::string& fname,
	std::vector<std::string>& lines, int line_width, bool include_fname)
{
	// first attempt to extract the file name
	size_t pos1 = msg.find("[");
	size_t pos2 = msg.find("]");
	std::string pure_msg;
	fname.clear();
	if (pos1 == 0 && pos2 != std::string::npos && pos1 < pos2) {
		size_t pos3 = msg.find_first_not_of(" \t", pos2+1);
		if (pos3 != std::string::npos) {
			fname = msg.substr(0, pos3);
			pure_msg = msg.substr(pos3, std::string::npos);
		}
	}
	if (fname.empty())  	pure_msg = msg;
	// now fname contains the function name, e.g. "[parse_string]"
	// and pure_msg contains the message itself.

	// now we correct the line_width if the fname should be included
	if (!fname.empty()) {
		line_width = std::max<int>(static_cast<int>(fname.size()) + 20, line_width - static_cast<int>(fname.size()) - 2);
	}

	// now break the pure msg into substrings
	break_string(pure_msg, lines, line_width);

	if (include_fname && !fname.empty()) {
		for (unsigned int i=0; i<lines.size(); ++i) {
			// if line is empty do not prepend name
			if (i != lines.size()-1 || !lines[i].empty())
				lines[i] = fname + "  " + lines[i];
		}
	}
}
// ---------------------------------------------------------------------------

int count_tokens(const std::string& str) {
	std::stringstream strm(str);
	std::string dummy;
	int count = 0;
	while (strm >> dummy) ++count;
	return count;
}
// ---------------------------------------------------------------------------

/*! Checks if the given char is of the given type.
	\param ch Character for check.
	\param types Or combination of CharType enums.
*/
bool isInCharType(char ch, unsigned int types) {
	if( types & CT_LowerCaseASCII) {
		if( ch >= 97 && ch <= 122)
			return true;
	}
	if( types & CT_UpperCaseASCII) {
		if( ch >= 65 && ch <= 90)
			return true;
	}
	if( types & CT_Numbers) {
		if( ch >= 48 && ch <= 57)
			return true;
	}
	if( types & CT_SpecialCharsASCII) {
		if( ch >= 33 && ch <= 47)
			return true;
		if( ch >= 58 && ch <= 64)
			return true;
		if( ch >= 91 && ch <= 96)
			return true;
		if( ch >= 123 && ch <= 126)
			return true;
	}
	return false;
}

/*! Functor for creating a random character in the given chart type range.*/
struct random_char {
public:
	/*! Constructor set the char type.*/
	random_char(unsigned int charTypes) :
		m_charTypes(charTypes)
	{}
	/*! Functor for use in generators.
		Creates a random char in the given range.*/
	char operator()() {
		if( m_charTypes == 0 || m_charTypes > 15)
			return '\0';
		int r;
		do {
			int rr = std::rand();
			r = rr % 90 + 33;
		} while ( !isInCharType((char)r, m_charTypes));
		return (char)r;
	}
private:
	/*! Used char types. Or combination of CharTypes enum.*/
	unsigned int m_charTypes;
};


std::string random_string(size_t length, unsigned int charTypes) {
	std::string res;
	if( length == 0 || charTypes == 0)
		return res;

	static bool first = true;
	if( first ) {
		std::srand(static_cast<unsigned int>(std::time(nullptr)));
		first = false;
	}
	std::generate_n(std::back_inserter(res), length, random_char(charTypes));
	return res;
}


std::string read_until_char_cut( std::string &src, const char ch)
{
	std::size_t pos = src.find(ch);
	std::string dest;
	if( pos != std::string::npos) {
		dest = src.substr(0,pos);
		src.erase(0,pos+1);
	}
	return dest;
}


std::string replace_string(const std::string& src, const std::string& old_pattern,
		const std::string& new_pattern, StringReplaceKind flag)
{
	if (old_pattern == new_pattern)
		return src;
	std::string resstr(src);
	switch (flag) {
		case ReplaceNone:
			return src;
		case ReplaceAll: {
			std::string::size_type pos = 0;
			while ((pos = resstr.find(old_pattern, pos)) != std::string::npos ) {
				resstr = resstr.replace(pos, old_pattern.size(), new_pattern);
				pos += new_pattern.size();
			}
			break;
		}
		case ReplaceFirst: {
			std::string resstr(src);
			std::string::size_type pos(resstr.find(old_pattern));
			if (pos != std::string::npos )
				return resstr.replace(pos, old_pattern.size(), new_pattern);
			break;
		}
		case ReplaceLast: {
			std::string::size_type pos, lpos(std::string::npos);
			std::string resstr(src);
			while ((pos = resstr.find(old_pattern)) != std::string::npos )
				if( pos != std::string::npos ) lpos = pos;
			if ( lpos != std::string::npos )
				resstr = resstr.replace(lpos, old_pattern.size(), new_pattern);
			break;
		}
	}
	return resstr;
}


std::string delete_chars(const std::string& src, const std::string& pattern) {
	std::string res(src);
	std::string::size_type pos;
	while((pos = res.find_first_of(pattern)) != std::string::npos )
		res = res.replace(pos, 1, "");
	return res;
}


std::string tolower_string(std::string input) {
#if defined(__BORLANDC__)
#if (__BORLANDC__ <= 0x570)
	// BCB 5 & 6
	std::use_facet<std::ctype<std::string::value_type> >(std::locale()).tolower(input.begin(), input.end());
#else
	typedef std::ctype< char > char_type;
	char_type const& the_type_ptr = std::use_facet<char_type>( std::locale() );
	for ( std::string::size_type i = 0; i < input.size(); ++i ) {
		input[i] = the_type_ptr.tolower( input[i] );
	}

#endif // (__BORLANDC__ <= 0x570)
#elif defined(__MINGW32__)
	std::transform(input.begin(), input.end(), input.begin(), ::tolower );
#elif defined(_MSC_VER)
	std::transform(input.begin(), input.end(), input.begin(), ::tolower );
#else
	// works for __GNUC__
	std::transform(input.begin(), input.end(), input.begin(), ::tolower );
#endif
	return input;
}


std::string toupper_string(std::string input) {
#if defined(__BORLANDC__)
#if (__BORLANDC__ <= 0x570)
	// BCB 5 & 6
	std::use_facet<std::ctype<std::string::value_type> >(std::locale()).toupper(input.begin(), input.end());
#else
	typedef std::ctype< char > char_type;
	char_type const& the_type_ptr = std::use_facet<char_type>( std::locale() );
	for ( std::string::size_type i = 0; i < input.size(); ++i ) {
		input[i] = the_type_ptr.toupper( input[i] );
	}
#endif // (__BORLANDC__ <= 0x570)
#elif defined(__MINGW32__)
	std::transform(input.begin(), input.end(), input.begin(), ::toupper );
#elif defined(_MSC_VER)
	std::transform(input.begin(), input.end(), input.begin(), ::toupper );
#else
	// works for __GNUC__
	std::transform(input.begin(), input.end(), input.begin(), ::toupper );
#endif
	return input;
}


std::string shorten_string(const std::string & src, unsigned int maxLength) {
	if (src.size() <= maxLength + 3)
		return src;

	double len = 0.5*(maxLength - 3);
	return src.substr(0, (std::size_t)std::ceil(len)) + "..." + src.substr(src.size() - (std::size_t)std::floor(len));
}


int levenshtein_distance(const std::string& source, const std::string& target) {
	// Step 1
	const unsigned int n = static_cast<unsigned int>(source.length());
	const unsigned int m = static_cast<unsigned int>(target.length());
	if (n == 0)
		return m;
	if (m == 0)
		return n;

	std::vector< std::vector<unsigned int> > matrix(n+1, std::vector<unsigned int>(m+1));
	// Step 2
	for (unsigned int i = 0; i <= n; i++)
		matrix[i][0]=i;
	for (unsigned int j = 0; j <= m; j++)
		matrix[0][j]=j;
	// Step 3
	for (unsigned int i = 1; i <= n; i++) {
		const char s_i = source[i-1];
		// Step 4
		for (unsigned int j = 1; j <= m; j++) {
			const char t_j = target[j-1];
			// Step 5
			unsigned int cost;
			if (s_i == t_j)
				cost = 0;
			else
				cost = 1;
			// Step 6
			const unsigned int above = matrix[i-1][j];
			const unsigned int left = matrix[i][j-1];
			const unsigned int diag = matrix[i-1][j-1];
			unsigned int cell = static_cast<unsigned int>(std::min(above + 1, std::min(left + 1, diag + cost)));

			// Step 6A: Cover transposition, in addition to deletion,
			// insertion and substitution.
			if (i>2 && j>2) {
				unsigned int trans=matrix[i-2][j-2]+1;
				if (source[i-2]!=t_j)
					trans++;
				if (s_i!=target[j-2])
					trans++;
				if (cell>trans)
					cell=trans;
			}
			matrix[i][j]=cell;
		}
	}
	// Step 7
	return matrix[n][m];
}


int levenshtein_distance_min(const std::string& source, const std::string& target) {
	size_t s1(source.size()), s2(target.size());
	std::vector<int> dists;
	if( s1 <= s2)
	{
		size_t diff = s2 - s1;
		for( size_t i=0; i<=diff; ++i)
		{
			dists.push_back(IBK::levenshtein_distance(source, target.substr(i, s1)));
		}
	} else
	{
		size_t diff = s1 - s2;
		for( size_t i=0; i<=diff; ++i)
		{
			dists.push_back(IBK::levenshtein_distance(source.substr(i, s2), target));
		}
	}
	return *std::min_element(dists.begin(), dists.end());
}

bool is_valid_utf8_string(const std::string& str) {
	return utf8::is_valid(str.begin(), str.end());
}

//---------------------------------------------------------------------------

unsigned short HexToShort(char ch) {
	if( ch>=65 && ch<=70)
		return ch-55;
	if( ch>=97 && ch<=102)
		return ch-87;
	if( ch>=48 && ch<=57)
		return ch-48;
	throw IBK::Exception("Given char does not name a hex!","[HexToShort]");
}

unsigned short Hex2DToShort(char upper, char lower) {
	return HexToShort(upper) * 16 + HexToShort(lower);
}

void decode_version_number(const std::string & versionString, unsigned int & major, unsigned int & minor,
						   unsigned int & patch)
{
	FUNCID(decode_version_number);
	try {
		std::vector<std::string> versionTokens;
		IBK::explode(versionString, versionTokens, '.', true);
		if (versionTokens.size() > 2)	patch = string2val<unsigned int>(versionTokens[2]);
		else							patch = 0;
		if (versionTokens.size() > 1)	minor = string2val<unsigned int>(versionTokens[1]);
		else							minor = 0;
		if (versionTokens.size() > 0)	major = string2val<unsigned int>(versionTokens[0]);
		else							major = 0;
	}
	catch (IBK::Exception &) {
		throw IBK::Exception(IBK::FormatString("Cannot parse version string '%1'").arg(versionString), FUNC_ID);
	}
}


std::string convertXml2Html(const std::string & xmlText) {
	static std::vector< std::pair<std::string, std::string> >substitutionTable;
	if (substitutionTable.empty()) {
		// Note: & must be replaced at the very begin!
		substitutionTable.push_back( std::make_pair("&", "&amp;") );	//	ampersand
		substitutionTable.push_back( std::make_pair("\"","&quot;") ); //	quotation mark
//		substitutionTable["'"]	= "&apos;"; //	apostrophe
		substitutionTable.push_back( std::make_pair("<","&lt;") ); //	less-than
		substitutionTable.push_back( std::make_pair(">","&gt;") ); //	greater-than
		substitutionTable.push_back( std::make_pair("\n", "<br>") ); //	line break
//		substitutionTable.push_back( std::make_pair(" ", "&nbsp;") ); //	no-break-space
	}

	std::string newText = xmlText;
	for (const auto & subst : substitutionTable)
		newText = replace_string(newText, subst.first, subst.second);

	return newText;
}


#ifdef _WIN32

std::wstring UTF8ToWstring(const std::string& utf8str) {
	FUNCID(IBK::UTF8ToWstring);
	if (utf8str.empty())
		return std::wstring();

	int reslength = MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), -1, 0, 0);
	if (reslength == 0)
		throw IBK::Exception("Cannot create wide string from UTF8 string.", FUNC_ID);

	std::vector<wchar_t> wide(reslength, L'\0');
	int writtenLength = MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), -1, &wide[0], reslength);
	if (writtenLength == 0)
		throw IBK::Exception("Cannot create wide string from UTF8 string.", FUNC_ID);

	return std::wstring(&wide[0]);
}


std::string WstringToUTF8(const std::wstring& wide) {
	FUNCID(IBK::WstringToUTF8);
	if (wide.empty())
		return std::string();

	int reslength = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, 0, 0, 0, 0);
	if (reslength == 0)
		throw IBK::Exception("Cannot create UTF8 string from wide string.", FUNC_ID);

	std::vector<char> str(reslength, '\0');
	int writtenLength = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, &str[0], reslength, 0, 0);
	if (writtenLength == 0)
		throw IBK::Exception("Cannot create UTF8 string from wide string.", FUNC_ID);
	return std::string(&str[0]);
}


std::wstring ANSIToWstring(const std::string& ansiString, bool OEMPage) {
	FUNCID(IBK::ANSIToWstring);
	if (ansiString.empty())
		return std::wstring();

	DWORD pageFlag = OEMPage ? CP_OEMCP : CP_ACP;
	int reslength = MultiByteToWideChar(pageFlag, 0, ansiString.c_str(), -1, 0, 0);
	if (reslength == 0)
		throw IBK::Exception("Cannot create wide string from Ansi (local encoding) string.", FUNC_ID);

	std::vector<wchar_t> wide(reslength, L'\0');
	int writtenLength = MultiByteToWideChar(pageFlag, 0, ansiString.c_str(), -1, &wide[0], reslength);
	if (writtenLength == 0)
		throw IBK::Exception("Cannot create wide string from Ansi (local encoding) string.", FUNC_ID);

	return std::wstring(&wide[0]);
}


std::string WstringToANSI(const std::wstring& wide, bool OEMPage) {
	if(wide.empty())
		return std::string();

	DWORD pageFlag = OEMPage ? CP_OEMCP : CP_ACP;
	int reslength = WideCharToMultiByte(pageFlag, 0, wide.c_str(), -1, 0, 0, 0, 0);
	if(reslength == 0)
		throw std::logic_error("Cannot create wide string from UTF8 string.");

	std::vector<char> str(reslength, '\0');
	int writtenLength = WideCharToMultiByte(pageFlag, 0, wide.c_str(), -1, &str[0], reslength, 0, 0);
	if(writtenLength == 0)
		throw std::logic_error("Cannot create ansi string from wide string.");
	return std::string(&str[0]);
}

#endif // _WIN32

}  // namespace IBK

