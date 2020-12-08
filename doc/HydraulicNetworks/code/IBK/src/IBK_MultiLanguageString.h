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

#ifndef IBK_MultiLanguageStringH
#define IBK_MultiLanguageStringH

#include <map>
#include <vector>
#include <set>
#include <string>

namespace IBK {

/*! \brief Class contains multi language strings.

	This class can be used to parse/write language specific strings. The strings
	are stored in UTF8 encoding which allows us to use plain std::string operations
	throughout the class. All functions returns strings should be treated as UTF8 encoding
	in the client code, e.g. in a user interface that displays the strings.

	The different strings are stored in the mapping m_taglist. It holds in addition the full
	string, mapped to the key "all". This full string is updated whenever
	the tag lists change, e.g. via a call to setString().

	The class holds a static property m_language as default language that is used as selector
	for the language when not specified otherwise.
*/
class MultiLanguageString {
public:

	/*! Constructor, creates a MultiLanguageString object from a given string.
		\param encodedString A string either without language ID or as full encoded string.
	*/
	explicit MultiLanguageString(const std::string& encodedString = "");

	/*! Sets the content of the multi-language-string from encoded string.
		\param encodedString A string in format "de:Deutscher Text|en:english text".

		\note You can also set a string without encoding, that is without language ID.
	*/
	void setEncodedString(const std::string & encodedString);

	/*! Returns the content of the multi-language-string in encoded form.
		\return A string in format "de:Deutscher Text|en:english text".

		\note If a string was set without language Id, it will be returned without language Id as well.
	*/
	const std::string & encodedString() const { return m_taglist.find("all")->second; }

	/*! Read encoded string in binary format from input stream. */
	void readBinary(std::istream& in);

	/*! Writes encoded string in binary format to the given output stream. */
	void writeBinary(std::ostream& out) const;

	/*! Sets a string in the given language.
		This also updates the m_taglistS["all"] value.
		If the string was previously initialized with an invalid content ("all" set, but both
		taglists empty), the taglists and the "all" value will be overwritten.
	*/
	void setString(const std::string& str, const std::string& languageId);

	/*! Returns a string in a specific language.
		If the specific language doesn't exist an empty string will be returned.
		\param languageId The language Id, that the string should be returned for. If missing,
						  the default language is being used.
		\param returnAllString If a string with given language is not found and returnAllString
							   is true, the encoded (all) string is returned instead of an empty string.
		\code
		MultiLanguageString str("de:Ein Text | en:a text");
		// get german text
		std::string destr = str.string("de");

		// use default language
		MultiLanguageString::m_language = "en";
		std::string enstr = str.string();
		\endcode

		\warning You must not use 'all' as languageId (otherwise an IBK::Exception is thrown).
	*/
	std::string string(const std::string& languageId = m_language, bool returnAllString=false) const;

	/*! Clears all internal member variables.*/
	void clear();

	/*! Checks if the string is empty.
		\return Returns whether the full string ("all" in short taglist) is empty.
	*/
	bool empty() const {
		return m_taglist.find("all")->second.empty();
	}

	/*! Returns a vector of all languages currently used in the string.
		For all non-empty strings the language Id is returned in the vector.
		The language IDs for empty strings are not returned.
	*/
	std::vector<std::string> languages() const;

	/*! Inserts all languages currently used in the string into the set with language IDs.
		This is a convenience function and uses languages() internally.
	*/
	void languages(std::set<std::string> & languageIDs) const;

	/*! Returns true if the given language is currently used (a non-empty string is stored with
		this language Id). */
	bool hasLanguage(const std::string& language) const;

	/*! Returns true if languages (language identifiers) exists.
		This is a convenience function and uses languages() internally.
	*/
	bool hasLanguages() const { return !languages().empty(); }



	// *** STATIC MEMBER FUNCTIONS ***

	/*! Extracts language specific tags from a string with the format 'de:blablab|en:balbalba|it:balfd'.
		Different language strings including their language tags are separated by a | character.
		The language tag can be:
		- a two char string followd by a colon (en:)
		- the string "all" (means all languages)
		The tags are stored in the maps and can be accessed via tags["en"] etc.
		The language id tags are always lower case, independent of the case in the input string!
		The maps are cleared before parsing the string.
		\code
		string langIdString = "en:some text|de:irgendein Text";
		map<string,string> tags;
		explodeLangIDString(langIdString, tags);
		// tags["en"] = "some text";
		// tags["de"] = "irgendein Text";

		langIdString = "irgendein Text ohne tag";
		explode_langid_string(langIdString, tags);
		// tags["all"] = "irgendein Text ohne tag";
		\endcode
	*/
	static void explodeLangIDString(const std::string& input,
		std::map<std::string,std::string>& tags);

	/*! Creates a string from languages dependend list.
		Composes a string from the tags in the map.
		\code
		// tags["en"] = "some text";
		// tags["de"] = "irgendein Text";
		string langIDString = MultiLanguageString::composeLangIDString(tags, false);
		// langIDString = "en:some text |de:irgendein Text"
		langIDString = MultiLanguageString::composeLangIDString(tags, true);
		// langIDString = "en_l:some text |de_l:irgendein Text"
		\endcode
		This is a helper function used within MultiLanguageString, but is provided as
		public interface to allow more general use.
	*/
	static std::string composeLangIDString(const std::map<std::string,std::string>& tags);


	/*! Holds language ID of currently selected language.
		This variable will affect the comparison operations (MultiLanguageString and
		std::string) and the function string().
	*/
	static std::string					m_language;

private:
	/*! Composes the full string from the tag lists.
		Calls composeLangIDString() for m_taglist and concatenates
		generated strings.
		Sets the resulting string in m_taglist["all"].
	*/
	void createAllString();

	/*! Holds language specific strings.
		Maps a language ID "en" or "de", etc. to a string.
		Special value is "all" which holds the original string
		passed to the explodeLangIDString() function.
		*/
	std::map<std::string,std::string>	m_taglist;


}; // class IBK::MultiLanguageString



/*! Compares two MultiLanguageStrings.
	They are identical if all strings match.
*/
inline bool operator==(const MultiLanguageString& lhs, const MultiLanguageString& rhs) {
	return lhs.encodedString() == rhs.encodedString();
}

/*! Compares a MultiLanguageString with a std::string.
	This function test only for the default language.
*/
inline bool operator==(const MultiLanguageString& lhs, const std::string& rhs) {
	return lhs.string() == rhs;
}

/*! Compares a std::string with a MultiLanguageString.
	This function test only for the default language.
*/
inline bool operator==(const std::string& rhs, const MultiLanguageString& lhs) {
	return rhs == lhs.string();
}

/*! Comparison operator. */
inline bool operator!=(const MultiLanguageString& lhs, const MultiLanguageString& rhs) {
	return !(lhs == rhs);
}

/*! Compares a MultiLanguageString with a std::string. */
inline bool operator!=(const MultiLanguageString& lhs, const std::string& rhs) {
	return !(lhs == rhs);
}

/*! Compares a std::string with a MultiLanguageString. */
inline bool operator!=(const std::string& lhs, const MultiLanguageString& rhs) {
	return !(lhs == rhs);
}

/*! Operator less for two MultiLanguageStrings, sorts based on strings of
	currently selected language.
*/
inline bool operator<(const MultiLanguageString& lhs, const MultiLanguageString& rhs) {
	return lhs.string() < rhs.string();
}

/*! Reads an MultiLanguageString in UTF8 encoding.
	The input stream is expected to have an endline character after the multi-line string
	(we read with getline()).
*/
std::istream& operator>>(std::istream& in, MultiLanguageString& val);

/*! Writes an MultiLanguageString string as encoded string.*/
std::ostream& operator<<(std::ostream& out, const MultiLanguageString& val);

} // namespace IBK

/*! \file IBK_MultiLanguageString.h
	\brief Contains the declaration of class MultiLanguageString.
*/

#endif // IBK_MultiLanguageStringH
