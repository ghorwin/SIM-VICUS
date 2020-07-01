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

#include "IBK_MultiLanguageString.h"

#include <algorithm>

#include "IBK_StringUtils.h"
#include "IBK_InputOutput.h"
#include "IBK_assert.h"

namespace IBK {

// *** STATIC MEMBER VARIABLE DEFINITIONS ****
std::string MultiLanguageString::m_language = "en";


MultiLanguageString::MultiLanguageString(const std::string& encodedString) {
	setEncodedString(encodedString);
}
// ----------------------------------------------------------------------------


void MultiLanguageString::setEncodedString(const std::string & encodedString) {
	explodeLangIDString(encodedString, m_taglist);
	// also set original string as "all" element
	m_taglist["all"] = encodedString;
}
// ----------------------------------------------------------------------------


std::string MultiLanguageString::string(const std::string& languageId, bool returnAllString) const {
	std::string lang = IBK::tolower_string(languageId);
	// ensure that "all" is not used as languageId
	if (lang=="all")
		throw IBK::Exception("Cannot use 'all' as language ID for MultiLanguageString.", "[MultiLanguageString::string]");
	std::map<std::string,std::string>::const_iterator fit = m_taglist.find(lang);
	if (fit == m_taglist.end()) {
		// if returnAllString is set, return original full string if only "all" is set
		if (returnAllString)
			return m_taglist.find("all")->second;
		return "";
	}
	return fit->second;
}
// ----------------------------------------------------------------------------


void MultiLanguageString::setString(const std::string& str, const std::string& languageId) {
	std::string lang = IBK::tolower_string(languageId);
	// setting the "all" string is prohibited
	if (lang=="all")
		throw IBK::Exception("Cannot use 'all' as language ID for MultiLanguageString.", "[MultiLanguageString::setString]");
	std::map<std::string,std::string>::iterator fit = m_taglist.find(lang);
	std::string trimmedString = IBK::trim_copy(str);
	if (fit != m_taglist.end()) {
		// remove language ID tag from map if string is empty (clears the language string)
		if (trimmedString.empty())
			m_taglist.erase(fit);
		else
			fit->second = str;
	}
	else {
		if (!trimmedString.empty())
			m_taglist[lang] = str;
	}
	createAllString();
}
// ----------------------------------------------------------------------------


void MultiLanguageString::clear() {
	m_taglist.clear();
	m_taglist["all"] = ""; // ensure, that key 'all' is defined!
}
// ----------------------------------------------------------------------------


void MultiLanguageString::readBinary(std::istream& in){

	size_t tmpSizeT;
	in.read(reinterpret_cast<char *>(&tmpSizeT), sizeof(size_t));

	for ( size_t i = 0; i < tmpSizeT; ++i ){
		std::string first,second;
		IBK::read_string_binary( in, first, 10000 );
		IBK::read_string_binary( in, second, 10000 );
		m_taglist[first] = second;
	}

}
// ----------------------------------------------------------------------------


void MultiLanguageString::writeBinary(std::ostream& out ) const {

	// write index count
	size_t count = m_taglist.size();
	out.write(reinterpret_cast<const char *>(&count), sizeof(size_t));

	// write s map
	for(  std::map< std::string, std::string >::const_iterator it = m_taglist.begin(), end = m_taglist.end(); it != end; ++it ){

		IBK::write_string_binary(out, it->first);
		IBK::write_string_binary(out, it->second);

	}

}
// ----------------------------------------------------------------------------


bool MultiLanguageString::hasLanguage(const std::string& language) const {
	std::string lang = IBK::tolower_string(language);
	if( lang == "all")
		return false;
	std::map<std::string,std::string>::const_iterator fit = m_taglist.find(lang);
	return fit != m_taglist.end();
}
// ----------------------------------------------------------------------------


std::vector<std::string> MultiLanguageString::languages() const {
	std::set<std::string> langIDs;
	languages(langIDs);
	std::vector<std::string> langs(langIDs.begin(), langIDs.end());
	return langs;
}
// ----------------------------------------------------------------------------


void MultiLanguageString::languages(std::set<std::string> & languageIDs) const {
	// first short strings
	for (std::map<std::string,std::string>::const_iterator it = m_taglist.begin();
		it != m_taglist.end(); ++it)
	{
		if( it->first != "all")
			languageIDs.insert(it->first);
	}
}
// ----------------------------------------------------------------------------


// *** PRIVATE FUNCTIONS ***

void MultiLanguageString::createAllString() {
	std::string str = composeLangIDString(m_taglist);
	if (str.empty())
		return;

	m_taglist["all"] = str;
}
// ----------------------------------------------------------------------------


// *** STATIC FUNCTIONS ***

void MultiLanguageString::explodeLangIDString(
	const std::string& input,
	std::map<std::string,std::string>& tags)
{
	// at first explode string at '|' characters
	tags.clear();
	if (input.empty())
		return;
	std::list<std::string> tokens;
	IBK::explode(input, tokens, '|');
	std::transform(tokens.begin(), tokens.end(), tokens.begin(), IBK::trimmer() );
	// now extract language ids from each token
	for (std::list<std::string>::const_iterator it = tokens.begin(); it != tokens.end(); ++it) {
		if (it->size() <= 2) {
			// malformed string, or no language tag, just store the original string
			tags.clear();
			tags["all"] = input;
			return;
		}
		std::string::size_type pos = it->find(":");
		if (pos != std::string::npos) {
			std::string langID = IBK::tolower_string(it->substr(0, pos));
			std::string text = it->substr(pos+1);
			IBK::trim(text);
			// if trimmed text is empty, skip this token
			if (text.empty())
				continue;

			// must be a two-char langID tag
			if (langID.size() != 2) {
				// we keep the original string, obviously the string is malformed
				tags.clear();
				tags["all"] = input;
				return; // exit function because of error
			}
			tags[langID] = text;
		}
	}
}
// ----------------------------------------------------------------------------


std::string MultiLanguageString::composeLangIDString(const std::map<std::string,std::string>& tags) {
	std::string tmp;
	for (std::map<std::string,std::string>::const_iterator it = tags.begin();
		it != tags.end(); ++it)
	{

		// skip "all" tag
		if (it->first == "all")
			continue;

		// there must not be empty tags, unless we have a programming error
		IBK_ASSERT(!it->second.empty());

		if (!tmp.empty())
			tmp += " |";
		tmp += IBK::toupper_string(it->first);
		tmp += ": ";
		tmp += it->second;
	}
	return tmp;
}
//---------------------------------------------------------------------------


// *** GLOBAL FUNCTIONS ***

std::ostream& operator<<(std::ostream& out, const MultiLanguageString& val) {
	out << val.encodedString();
	return out;
}
// ----------------------------------------------------------------------------


std::istream& operator>>(std::istream& in, MultiLanguageString& val) {
	std::string tstr;
	if (getline(in, tstr))
		val.setEncodedString(tstr);
	return in;
}
// ----------------------------------------------------------------------------



} // namespace IBK
