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

#include "IBK_Flag.h"

#include <stdexcept>
#include <iostream>
#include <iomanip>

#include "IBK_StringUtils.h"
#include "IBK_FormatString.h"
#include "IBK_Exception.h"

namespace IBK {

void Flag::set(const std::string& n, const std::string& s) {
	std::string lowerCaseState = IBK::tolower_string(s);
	m_name = n;
	if (lowerCaseState == "1" || lowerCaseState == "yes" || lowerCaseState == "true") {
		m_state = true;
	}
	else if (lowerCaseState == "0" || lowerCaseState == "no" || lowerCaseState == "false") {
		m_state = false;
	}
	else
		throw IBK::Exception( IBK::FormatString("Invalid value '%2' for flag '%1'!").arg(m_name).arg(s),"[Flag::set]" );
}


void Flag::setOrClear(const std::string& n, bool s) {
	m_state = s;
	if (s)	m_name = n;
	else 	m_name.clear();
}


void Flag::read(std::istream& in) {
	std::string name_;
	std::string state_;
	in >> name_ >> state_;
	if (!in)
		throw IBK::Exception( "Error reading flag!","[Flag::read]");
	if (state_ == "1" || string_nocase_compare(state_, "yes") || string_nocase_compare(state_, "true") ) {
		m_state = true;
	}
	else if (state_ == "0" || string_nocase_compare(state_, "no") || string_nocase_compare(state_, "false") ){
		m_state = false;
	}
	else
		throw IBK::Exception( IBK::FormatString("Invalid value for flag '%1' !") .arg(name_),"[Flag::read]" );
}



void Flag::write(std::ostream& out, unsigned int indent, unsigned int width) const {
	out << std::string(indent, ' ');
	out << std::setw(width) << std::left << m_name << " = ";
	if (m_state) 	out << "yes";
	else		out << "no";
}



bool Flag::operator!=(const Flag & other) const {
	if (m_state != other.m_state) return true;
	if (m_name != other.m_name) return true;
	return false; // this and other hold the same data
}


} // namespace IBK
