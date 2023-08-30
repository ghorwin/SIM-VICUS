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

#ifndef IBK_StringUtilsH
#define IBK_StringUtilsH

#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <list>
#include <vector>
#include <typeinfo>
#include <map>
#include <limits>
#include <locale>

#include "IBK_Exception.h"


#ifdef _MSC_VER
#define TOLOWER(x) tolower(x)
#else
/// A macro that wraps std::tolower() and tolower() on Windows API
#define TOLOWER(x) std::tolower(x)
#endif

namespace IBK {

/*! Result code for the extractFromParenthesis() function. */
enum ExtractResultType {
	ERT_Success,
	ERT_NoParenthesis,
	ERT_NoNumber,
	ERT_BadNumber
};

/*! Enum characterizes different types of chars (area in ASCII table).
	It is mainly used for generating random chars.
*/
enum CharTypes {
	CT_LowerCaseASCII		= 1,	///< Lower case characters (ASCII 97 - 122).
	CT_UpperCaseASCII		= 2,	///< Upper case characters (ASCII 65 - 90).
	CT_Numbers				= 4,	///< Number characters (ASCII 48 - 57).
	CT_SpecialCharsASCII	= 8		///< Special printable characters (ASCII 33 - 47; 58 - 64; 91 - 96; 123 - 126).
};

/*! Flags for explode() function */
enum ExplodeFlags {
	EF_NoFlags          = 0x00,
	EF_TrimTokens       = 0x01,
	EF_KeepEmptyTokens  = 0x02,
	EF_UseQuotes        = 0x04
};

/*! Enumeration types used in replace_string(). */
enum StringReplaceKind {
	ReplaceNone,
	ReplaceAll, 	///< To replace all occurences.
	ReplaceFirst,	///< To replace only the first occurence of the string.
	ReplaceLast		///< To replace only the last occurence of the string.
};

// windows specific conversion functions
#if defined(_WIN32)

/*! Converts a given UTF8 encoded string into a unicode string.*/
std::wstring UTF8ToWstring(const std::string& utf8str);

/*! Converts a given unicode string into a UTF8 encoded string.*/
std::string WstringToUTF8(const std::wstring& wide);

/*! Converts the given ANSI string to a wide string (UTF16).
	The given ANSI string must be encoded with the standard codepage or the OEM codepage.*/
std::wstring ANSIToWstring(const std::string& ansiString, bool OEMPage);

/*! Converts the given wide string (UTF16) to a ANSI string.
	The given ANSI string will be encoded with the standard codepage or the OEM codepage.*/
std::string WstringToANSI(const std::wstring& wide, bool OEMPage);

/*! Converts a given UTF8 string into a codepage string.*/
inline std::string UTF8ToANSIString(const std::string& utf8str) {
	return  WstringToANSI(UTF8ToWstring(utf8str), false);
}

inline std::string ANSIToUTF8String(const std::string& ansiString) {
	return WstringToUTF8(ANSIToWstring(ansiString, false));
}

#endif


/*! Converts the value 'val' to a string. */
template <class T>
std::string val2string(const T val) {
	if (!(val == val)) { //-V501
		return "1.#QNAN";
	}
	std::stringstream strm;
	strm << val;
	return strm.str();
}


/*! Converts the boolean 'val' to a string. */
template <>
inline std::string val2string<bool>(const bool val) {
	return (val ? "true" : "false");
}


/*! Converts the value 'val' to a string with given precision. */
template <class T>
std::string val2string(const T val, const int precision) {
	if (!(val == val)) { //-V501
		return "1.#QNAN";
	}
	std::stringstream strm;
	strm.precision(precision);
	strm << val;
	return strm.str();
}

/*! Converts the value 'val' to a string using special format qualifiers (total string width and fill chars). */
template <class T>
std::string val2string(const T val, const std::size_t width, const char fillchar) {
	if (!(val == val)) { //-V501
		return "1.#QNAN";
	}
	std::stringstream strm;
	strm.fill(fillchar);
	strm << std::setw(width) << val;
	return strm.str();
}

/*! Converts the value 'val' to a string using special format qualifiers (precision, total string width and fill chars). */
template <class T>
std::string val2string(const T val, const int precision, const std::size_t width, const char fillchar) {
	if (!(val == val)) { //-V501
		return "1.#QNAN";
	}
	std::stringstream strm;
	strm.fill(fillchar);
	strm.precision(precision);
	strm << std::setw(width) << val;
	return strm.str();
}

/*! Attempts to extract a numerical value from a string.
	\code
	double val = string2val<double>("2.5");
	\endcode
*/
template <class T>
T string2val(const std::string& str) {
	T val;
	std::stringstream strm(str);
	if (!(strm >> val))
		throw IBK::Exception(IBK::FormatString("Could not convert '%1' into value.").arg(str), "[string2val]");
	return val;
}


template <>
double string2val<double>(const std::string& str);

template <>
bool string2val<bool>(const std::string& str);

/*! Attempts to extract a numerical value from a string.
	Returns the def value in case of non valid string.
	\code
	double val = string2val<double>("2.5", 0.0);
	\endcode
*/
template <class T>
T string2valDef(const std::string& str, const T& def) {
	std::stringstream strm(str);
	T val;
	if (!(strm >> val))
		return def;
	return val;
}

/*! Attempts to extract a numerical value from a string.
	Returns the def value in case of non valid string.
	\code
	double val = string2val<double>("2.5", 0.0);
	\endcode
*/
template <>
double string2valDef<double>(const std::string& str, const double & def);


/*! Converts a string with white-space separated values into a vector.
	The vector 'vec' is emptied upon start.

	\note The individual numbers are appended using push_back() to the vector. In order to
	improve performance, you may want to call reserve() on the vector if
	you know the approximate/exact number of numbers in the string.

	This function throws an IBK::Exception in case of invalid numbers in string.
*/
void string2valueVector(const std::string & str, std::vector<double> & vec);


/*! Tries to read a vector of numbers of type T out of a string.
	If the number of read values is less then 2, a runtime error is thrown.
*/
template <class T>
void string2vector(const std::string& data, std::vector<T>& vec) {
	std::stringstream strm(data);
	T para;
	vec.clear();
	while (strm >> para)
		vec.push_back(para);
}

/*! Converts a vector to a string representation. */
template <class T>
std::string vector2string(const std::vector<T>& vec, std::string separator = ", ") {
	std::stringstream strm;
	for (unsigned int i=0; i<vec.size()-1; ++i)
		strm << vec[i] << separator;
	strm << vec[vec.size()-1];
	return strm.str();
}

/*! Splits a line like " keyword = value" into "keyword" and "value".
	Returns 'true' if successful, or 'false' if not. In this case value==line
*/
bool extract_keyword(const std::string& line, std::string& keyword, std::string& value);

/*! Extracts a value from paranthesis within a string, "blabla(15)". */
unsigned int extractFromParenthesis(const std::string & src, unsigned int defaultValue);


/*! Extracts a value from paranthesis within a string, "blabla(15)".
	\param src Source string of format "Band(12)" or "Band"
	\param str Leading part of the string until the first (
	\param val Value between parenthesis, if any (only if ERT_Success is returned).
	\return Returns ERT_Success, if () was part of the string and a value could be extracted.
	\sa ExtractResultType
*/
template <typename T>
ExtractResultType extractFromParenthesis(const std::string & src, std::string & str, T & val) {
	std::string::size_type pos = src.find("(");
	std::string::size_type pos2 = src.find(")");
	if (pos != std::string::npos && pos2 != std::string::npos) {
		str = src.substr(0,pos);
		if (pos2 > pos) {
			std::string substr = src.substr(pos+1, pos2-pos-1);
			try {
				val = IBK::string2val<T>(substr);
				return ERT_Success;
			}
			catch (...) {
				return ERT_BadNumber;
			}
		}
		else
			return ERT_NoNumber;
	}
	str = src;
	return ERT_NoParenthesis;
}


/*! Extracts a value from paranthesis within a string, "blabla(0.01)". */
std::pair<unsigned int, double> extractFromParenthesis(const std::string & src,
													std::pair<unsigned int, double> defaultValue);


/*! Reads in (\a src) until char (\a ch)
	(\a src) stores the first part
	\return Rest of the string
*/
std::string read_until_char_cut( std::string &src, const char ch);

/*! Trims the string on both sides (removes tabs and spaces).
	This is the inplace-version of the trim() function.
*/
void trim(std::string& str, const std::string& trimchars = " \t\r");

/*! Trims the string on both sides (removes tabs and spaces).
	This is the copy version of the trim() function.
*/
std::string trim_copy(const std::string& str, const std::string& trimchars = " \t\r");

/*! removes the trailing '=' from the keyword string and the brackets '[' and ']' around it. */
std::string& trim_keyword(std::string& keyword);

/*! Functor for trimming strings.
	For use in transform() in order to trim a container of strings.
	\code
	vector<string> strvec;
	// fill vector with data
	// now trim all strings
	std::transform(strvec.begin(), strvec.end(), strvec.begin(), trimmer());
	\endcode
*/
class trimmer {
public:
	/*! Evaluation operator, returns a trimmed copy of the string argument using default trim characters, see trim(). */
	std::string operator()(const std::string& val) {
		return IBK::trim_copy(val);
	}
};

/*! Removes comments from the str (removes everything including the ';'). */
std::string& remove_comment(std::string& str);

/*! Binary predicate that returns true when the characters are equal in lower case.
	Can be used in an algorithm to compare two strings case-insensitive.
	\code
	string vec1 = "blaBlaBLA";
	string vec2 = "blablabla";
	bool equal = std::equal(vec1.begin(), vec1.end(), vec2.begin(), nocase_equal());
	\endcode
*/
class nocase_equal {
public:
	/*! Evaluation operator, returns whether left uppercased character equals right upper cased character. */
	bool operator() (char lhs, char rhs) { return TOLOWER(lhs)==TOLOWER(rhs); }
};

/*! Compares two string case insensitive.
	\return Returns 'true' if strings are equal.
*/
bool string_nocase_compare(const std::string& lhs, const std::string& rhs);

/*! Compares two string case insensitive, and returns whether lhs is < than rhs
	\return Returns 'true' if left string is smaller than right string.
*/
bool string_nocase_less(const std::string& lhs, const std::string& rhs);

/*! Finds the string 'substr' in the string 'str' (case insensitive).
	Returns either the position of the substr or string::npos.
*/
std::size_t string_nocase_find(const std::string& str, const std::string& substr);

/*! Explodes the string 'str' into a vector of substrings, using the 'delimiter' as separation char.
	\warning This function is not really fast, because of the temporary vector created.

	\param str Original string
	\param delim Separation character
	\param maxTokens Maximum number of tokens to extract (i.e. max size of returned vector), rest is discarded.
*/
std::vector<std::string> explode(const std::string & str, char delim, int maxTokens = -1);

/*! Explodes the string 'str' into substrings (stored in the string list
	'tokens'), which were separated by 'delim' characters.
	\param str Original string
	\param tokens Vector with tokens
	\param delims Separation characters
	\param explodeFlags Combination of flags to control behaviour
	\sa ExplodeFlags
*/
size_t explode(const std::string& str, std::vector<std::string>& tokens, const std::string& delims, int explodeFlags);

/*! Explodes the string 'str' into substrings (stored in the string list
	'tokens'), which were separated by 'delim' characters.
	\param str Original string
	\param tokens List with tokens
	\param delim Separation character
	\param trimTokens If true, each token is passed through the trim() function (stripping all whitespaces around tokens)
	\deprecated
*/
size_t explode(const std::string& str, std::list<std::string>& tokens, char delim, bool trimTokens=false);

/*! Explodes the string 'str' into substrings (stored in the string list
	'tokens'), which were separated by 'delim' characters.
	\param str Original string
	\param tokens Vector with tokens
	\param delim Separation character, defaults to space
	\param trimTokens If true, each token is passed through the trim() function (stripping all whitespaces around tokens)
	\deprecated
*/
size_t explode(const std::string& str, std::vector<std::string>& tokens, char delim = ' ', bool trimTokens=false);


/*! Explodes the string 'str' into substrings (stored in the string list
	'tokens'), which were separated by any of the characters in the delimiter string.
	The actual delimeters are stored in the string delims.
	\deprecated
*/
size_t explode(const std::string& str, std::list<std::string>& tokens,
	std::string& delims, const std::string& delimiter);


/*! Explodes the string 'str' into max 2 substrings (stored in the string list
	'tokens'), which were separated by 'delim' characters.
	\param str The input string.
	\param tokens A vector, will be resized to 2.
	\param delim Delimiting character.
	\return Returns number of tokens extracted, 1 if the delimiter is missing (in this case the
			whole string is returned in tokens[0].
	\deprecated
*/
size_t explode_in2(const std::string& str, std::vector<std::string>& tokens, char delim = ' ');

/*! Explodes the string 'str' into max 2 substrings (stored in the string list
	'tokens'), which were separated by 'delim' characters.
	Works essentially like the function above, but allows specification of several separation characters.
	\param str The input string.
	\param tokens A vector, will be resized to 2.
	\param delim Delimiting characters.
	\return Returns number of tokens extracted, 1 if the delimiter is missing (in this case the
			whole string is returned in tokens[0].
	\deprecated
*/
size_t explode_in2(const std::string& str, std::vector<std::string>& tokens, const std::string & delims);

/*! Explodes the CSV (comma-separated-values) string 'str' into substrings stored
	in the string list 'tokens'.
	\deprecated
*/
void explode_csv(const std::string& str, std::list<std::string>& tokens);

/*! Explodes the CSV (comma-separated-values) string 'str' into substrings stored
	in the string vector 'tokens'.
	\deprecated
*/
void explode_csv(const std::string& str, std::vector<std::string>& tokens);

/*! Explodes a string using the same section title where data is given as block of endline-separated strings.*/
void explode_section(const std::string& str, const std::string& section_title,
	std::vector<std::string>& section_data);

/*! Explodes a string using the same section title, where data is given as vector of lines. */
void explode_section(const std::vector<std::string>& data, const std::string& section_title,
	std::vector<std::vector<std::string> >& section_data);

/*! Explodes content of an input stream using a list of known section titles.
	\code
	// suppose you have data with following content
	std::string data =	"[First Section]\n"
						"  text\n"
						"; A comment describing the second section"
						"[Second Section]\n"
						"  some more\n"
						"  text\n";
	// you can explode it into several parts by defining a vector with the section captions
	// without []
	std::vector<std::string> section_titles;
	section_titles.push_back("First Section");
	section_titles.push_back("Second Section");
	// creating a vector for the data, need not be resized (will be adjusted to size
	// in the function)
	std::vector<std::string> data;
	// now you can explode the data stream (by means of a temporary stringstream)
	std::stringstream strm(data);
	explode_section(strm, section_titles, data);
	// data will now hold:
	data[0] = "  text\n";
	data[1] = "  some more\n"
			  "  text\n";
	\endcode
*/
void explode_sections(std::istream& in,
					 const std::vector<std::string>& section_titles,
					 std::vector<std::string>& section_data,
					 const std::string & commentChars=";#");

/*! Explodes content of an input data vector using a list of known section titles.
	\code
	// suppose you have data with following content
	std::string data =	"[First Section]\n"
						"  text\n"
						"; A comment describing the second section"
						"[Second Section]\n"
						"  some more\n"
						"  text\n";
	// you can explode it into several parts by defining a vector with the section captions
	// without []
	std::vector<std::string> section_titles;
	section_titles.push_back("First Section");
	section_titles.push_back("Second Section");
	// creating a vector for the data, need not be resized (will be adjusted to size
	// in the function)
	std::vector<std::string> data;
	// now you can explode the data stream (by means of a temporary stringstream)
	std::stringstream strm(data);
	explode_section(strm, section_titles, data);
	// data will now hold:
	data[0] = "  text\n";
	data[1] = "  some more\n"
			  "  text\n";
	\endcode
*/
void explode_sections(const std::vector<std::string>& data,
					 const std::vector<std::string>& section_titles,
					 std::vector<std::vector<std::string> >& section_data,
					 const std::string & commentChars);

/*! Explodes a string using a list of known section titles.
	This is a convenience function around the previous explode_section() function.
*/
void explode_sections(const std::string& str, const std::vector<std::string>& section_titles,
	std::vector<std::string>& section_data);

/*! Explodes a string into several sections where each section is stored in a single line
	Essentially this function simply splits up a string into several lines which
	are stored in the vector section_data.
	\code
	const char * const data = "some line\nsome other line\nlast line";
	std::vector<std::string> sections;
	IBK::explode_lines(data, sections);
	// sections[0] = "some line";
	// sections[1] = "some other line";
	// sections[2] = "last line";

	// alternatively call
	explode(data, section, '\n');
	\endcode
*/
inline void explode_lines(const std::string& data, std::vector<std::string>& section_data) {
	explode(data, section_data, '\n');
}

/*! Joins a list of strings. */
template <typename T>
std::string join(T c, char sepChar = ' ') {
	std::string str;
	typename T::const_iterator it2;
	for (typename T::const_iterator it = c.begin(); it != c.end(); ++it) {
		str += *it;
		it2 = it; ++it2;
		if (it2 != c.end())
			str += sepChar;
	}
	return str;
}

/*! Joins a list of numbers (converting these into strings). */
template <typename T>
std::string join_numbers(const T & c, char sepChar = ' ') {
	std::string str;
	for (auto & t : c) {
		str += IBK::val2string(t);
		str += sepChar;
	}
	// remove trailing char
	if (!str.empty())
		str.erase(str.end()-1);
	return str;
}


/*! Returns a string with the formatted double number.
	Pass either ios_base::scientific or ios_base::fixed as optional third argument
	to specify fixed or exponential format.
	\todo merge with format string
*/
std::string format_double(double val, int precision, std::ios_base::fmtflags fmt = std::ios_base::floatfield, int width=0);

/*! Breaks the string 'msg' appart at the spaces and creates lines with
	max 'line_width' characters.
*/
void break_string(const std::string& msg, std::vector<std::string>& lines, int line_width);

/*! Counts the number of whitespace separated tokens in the string. */
int count_tokens(const std::string& str);

/*! Create a random string of (\a length).
	\param length Number of chars in the resulting string.
	\param charTypes Or combination of CharType enums used for string generation.
	charTypes = 0 returns an empty string.
	charTypes = 1 means only lower case characters from standard ASCII table.
	charTypes = 2 means only upper case characters from standard ASCII table.
*/
std::string random_string(size_t length, unsigned int charTypes);

/*! Converts string (\a src) into lowercase characters. */
std::string tolower_string(std::string src);

/*! Converts string (\a src) into lowercase characters. */
std::string toupper_string(std::string src);

/*! Reduces the size of the string to at max maxLength characters and adds an ellipsis (...)
	in the middle.
*/
std::string shorten_string(const std::string & src, unsigned int maxLength);

/*! Replace (\a oldPattern) by (\a newPattern) in (\a src)

	Example source code:
	\code
	string original = "my test string width wrong word width two wrong words";
	string new_str = IBK::replace_string(original, "width", "with", strtools::All);
	// new_str is now "my test string with wrong word with two wrong words"
	\endcode
	\param flag Defines replace mode:
				- ReplaceAll replace all occurences (default)
				- ReplaceFirst only first occurence
				- ReplaceLast only last occurence
	\param src Original string
	\param old_pattern Old substring to be replaced.
	\param new_pattern New substring to be used as replacement.
	\return resulting string
	\sa StringReplaceKind
*/
std::string replace_string(const std::string& src, const std::string& old_pattern,
	const std::string& new_pattern, StringReplaceKind flag = ReplaceAll);

/*! Deletes characters in \a pattern from \a src.
	\return Returns a copy of the string 'src' without the characters in 'pattern'.
*/
std::string delete_chars(const std::string& src, const std::string& pattern);

/*! Returns the Levenshtein distance of two strings.
	Usefull for implementation of a fuzzy search.
*/
int levenshtein_distance(const std::string& source, const std::string& target);

/*! Variant of Levenshtein distance of two strings.
	Returns the minimum of the real Levenshtein distance of the shoerter string inside the bigger one.
	Usefull for implementation of a fuzzy search.
*/
int levenshtein_distance_min(const std::string& source, const std::string& target);

/*! Checks whether the string \a str is a valid UTF-8 encoded string. */
bool is_valid_utf8_string(const std::string& str);

/*! Interprets the given char as integer.*/
unsigned short HexToShort(char ch);

/*! Interprets the given two digits of a hex char as integer.*/
unsigned short Hex2DToShort(char upper, char lower);


/*! Decodes a 3 part version number (example 6.1.15) into the respective version number parts. */
void decode_version_number(const std::string & versionString, unsigned int & major, unsigned int & minor,
						   unsigned int & patch);


/*! Converts windos file (line terminatation) into linux terminated stringstream.
	Returns true when finished successfull, false if files does not respond correctly.
	\todo to stringutils, operating on string buffer
*/
bool convertDosToUnix( const std::string& fname, std::stringstream& out, std::string& errmsg );

/*! Takes an XML text and encodes it in HTML by replacing special characters with symbols. */
std::string convertXml2Html(const std::string & xmlText);

}   // namespace IBK

/*! \file IBK_StringUtils.h
	\brief Contains utility functions for string operations.
*/

#endif // IBK_StringUtilsH
