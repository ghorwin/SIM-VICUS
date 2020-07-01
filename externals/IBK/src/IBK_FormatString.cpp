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
#include <sstream>

#include "IBK_FormatString.h"
#include "IBK_StringUtils.h"
#include "IBK_Unit.h"
#include "IBK_Path.h"

using namespace std;

namespace IBK {

FormatString::FormatString() {
}

FormatString::FormatString(const std::string& str) {
	this->operator+=(str);
}

FormatString::FormatString(const IBK::MultiLanguageString& str) {
	this->operator+=(str.string("en"));
}


void FormatString::clear() {
	m_tokens.clear();
	m_arguments.clear();
}

void FormatString::set(const std::string& str) {
	// same as clear and adding a string
	clear();
	this->operator+=(str);
}

std::string FormatString::str() const {
	// loop over all tokens and compose a string.
	string s;
	for (vector<FormatString::Token>::const_iterator it = m_tokens.begin(); it != m_tokens.end(); ++it) {
		// if the token string is empty, we have a placeholder
		if (it->token.empty()) {
			if (it->index == 0 || (it->index-1) >= m_arguments.size()) {
				// TODO : warning, invalid index, just substitute the text
				s += '%';
				s += val2string(it->index);
			}
			else {
				// look up the argument
				int i = it->index-1;
				s += m_arguments[i];
			}
		}
		else {
			// otherwise we have a normal string token which we add
			s += it->token;
		}
	}
	return s;
}

FormatString & FormatString::arg(const std::string & s, int fieldWidth) {
	if (fieldWidth == 0)
		m_arguments.push_back(s);
	else {
		std::stringstream strm;
		strm << setw(fieldWidth) << right << s;
		m_arguments.push_back(strm.str());
	}
	return *this;
}

FormatString & FormatString::arg(const IBK::MultiLanguageString & s, int fieldWidth) {
	if (fieldWidth == 0)
		m_arguments.push_back(s.string("en"));
	else {
		std::stringstream strm;
		strm << setw(fieldWidth) << right << s.string("en");
		m_arguments.push_back(strm.str());
	}
	return *this;
}

FormatString & FormatString::arg(const IBK::Path & path) {
	m_arguments.push_back(path.str());
	return *this;
}

FormatString & FormatString::arg(const IBK::Unit & unit) {
	m_arguments.push_back(unit.name());
	return *this;
}

FormatString & FormatString::arg(int i, int fieldWidth) {
	if (fieldWidth==0)
		m_arguments.push_back(val2string(i));
	else {
		std::stringstream strm;
		strm << setw(fieldWidth) << right << i;
		m_arguments.push_back(strm.str());
	}
	return *this;
}

FormatString & FormatString::arg(unsigned int i, int fieldWidth) {
	if (fieldWidth==0)
		m_arguments.push_back(val2string(i));
	else {
		std::stringstream strm;
		strm << setw(fieldWidth) << right << i;
		m_arguments.push_back(strm.str());
	}
	return *this;
}

#if !defined(_WIN32) || defined(_WIN64)
FormatString & FormatString::arg(size_t i, int fieldWidth) {
	if (fieldWidth==0)
		m_arguments.push_back(val2string(i));
	else {
		std::stringstream strm;
		strm << setw(fieldWidth) << right << i;
		m_arguments.push_back(strm.str());
	}
	return *this;
}
#endif

FormatString & FormatString::arg(double d, int fieldWidth, char format, int precision, const char & fillChar) {
	if (format == 'g')
		m_arguments.push_back(val2string(d));
	else {
		std::stringstream strm;
		if (format=='f')
			strm << std::fixed;
		else if (format=='e' || format=='E') {
			strm << std::scientific;
		}
		strm << std::setw(fieldWidth) << std::setprecision(precision) << std::setfill(fillChar);
		strm << d;
		m_arguments.push_back(strm.str());
	}
	return *this;
}

const FormatString & FormatString::operator+=(const std::string& s) {
	// here we do all the work with parsing the string and creating new tokens
	string current_token;
	for (unsigned int i=0; i<s.size(); ++i) {
		// do we have a new argument placeholder?
		if (s[i] == '%') {
			// store current token, unless it is empty
			if (!current_token.empty()) {
				m_tokens.push_back(FormatString::Token(current_token,0));
				current_token.clear();
			}
			// now start reading argument
			++i;
			// consider special case, two adjecent %%
			if (i < s.size() && s[i] == '%') {
				// simply start the new token with a single '%'
				current_token = '%';
				continue;
			}
			// must be a regular placeholder, extract the number
			string arg_placeholder;
			while (i < s.size() && s[i] >= 48 && s[i]<58) {
				arg_placeholder += s[i];
				++i;
			}
			if (arg_placeholder.empty()) {
				current_token = '%'; // just keep the character
			}
			else {
				// we convert the number in the arg placeholder
				int index = string2val<int>(arg_placeholder);
				// and store the token
				m_tokens.push_back(FormatString::Token(std::string(), index));
			}
			// if there was some character after the '%xx', read this character
			// again in the next loop
			if (i < s.size())
				--i;
		}
		else {
			// otherwise just add the character to the current token
			current_token += s[i];
		}
	}
	// add the current token unless it is empty
	if (!current_token.empty())
		m_tokens.push_back(FormatString::Token(current_token,0));
	return *this;
}

std::string FormatString::operator+(const std::string& s) const {
	return str() + s;
}

std::string operator+(const std::string& lhs, const FormatString& rhs) {
	// implement using operator+
	return rhs.operator+(lhs);
}

std::ostream& operator<<(std::ostream& strm, const FormatString& fstr) {
	return strm << fstr.str();
}

} // namespace IBK

