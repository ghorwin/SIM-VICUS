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

#include <stdexcept>
#include <iostream>
#include <iomanip>

#include "IBK_IntPara.h"
#include "IBK_InputOutput.h"
#include "IBK_Exception.h"
#include "IBK_StringUtils.h"

namespace IBK {

IntPara::IntPara(const std::string& str, int val)
	: name(str), value(val)
{
}


void IntPara::set(const std::string& str, int val) {
	name = str;
	value = val;
}


std::string IntPara::toString(bool without_name) const {
	std::stringstream strm;
	if (!without_name)
		strm << name << " = ";
	strm << value;
	return strm.str();
}


void IntPara::read(std::istream& in, bool no_name) {
	std::string name_, equal_;
	int val_;
	if (no_name)
		in >> val_;            // don't read name
	else
		in >> name_ >> equal_ >> val_;   // read name, too
	if (!in)
		throw IBK::Exception( "Error reading IntPara.", "[IntPara::read]");
	value = val_;
	if (!no_name)
		name = name_;        // assign read name
}


void IntPara::write(std::ostream& out, unsigned int indent, unsigned int paramWidth, bool writeName) const {
	out << std::string(indent, ' ');
	if (name.size() && writeName) {
		if (paramWidth > name.size())
			out << std::setw(paramWidth) << std::left << name << " = ";
		else
			out << name << " = ";
	}
	out << value << std::endl;
}


void IntPara::readBinary(std::istream& in) {
	read_string_binary(in, name, 10000);
	read_uint32_binary(in, (uint32_t&)value);
}


void IntPara::writeBinary(std::ostream& out) const {
	write_string_binary(out, name);
	write_uint32_binary(out, (uint32_t)value);
}

void IntPara::clear() {
	name.clear();
	value=0;
}


void IntPara::checkIfValueIsBelowLimit( const IBK::IntPara& limit, bool isLessEqual ) const {
	const char * const FUNC_ID = "[IntPara::checkIfValueIsBelowLimit]";

	// check IntPara
	if ( name.empty() || name != limit.name ) {
		std::string msg = limit.name + " is required";
		throw IBK::Exception(msg, FUNC_ID);
	}

	// test limits
	if (isLessEqual)
		test( limit, OP_LE);
	else
		test( limit, OP_LT);

}


void IntPara::checkIfValueIsAboveLimit( const IBK::IntPara& limit, bool isGreaterEqual ) const {
	const char * const FUNC_ID = "[IntPara::checkIfValueIsUpperBound]";

	// check IntPara
	if (name.empty() || name != limit.name ) {
		std::string msg = limit.name + " is required";
		throw IBK::Exception(msg, FUNC_ID);
	}

	// test limits
	if (isGreaterEqual)
		test( limit, OP_GE);
	else
		test( limit, OP_GT);
}


void IntPara::test( const IBK::IntPara& val, oper_t op ) const {

	static const char * const FUNC_ID = "[IntPara::test]";

	// convert and compare value
	switch (op) {
		case OP_GE :
			if ( !( value >= val.value ) ) {
				throw IBK::Exception(FormatString("IntPara '%1' out of range (must be >= %2)")
					.arg(name).arg(val.value), FUNC_ID);
			}
			break;

		case OP_GT :
			if (!(value > val.value)) {
				throw IBK::Exception(FormatString("IntPara '%1' out of range (must be > %2)")
					.arg(name).arg(val.value), FUNC_ID);
			}
			break;

		case OP_LE :
			if (!(value <= val.value)) {
				throw IBK::Exception( FormatString("IntPara '%1' out of range (must be <= %2)")
					.arg(name).arg(val.value), FUNC_ID);
			}
			break;

		case OP_LT :
			if (!(value < val.value)) {
				throw IBK::Exception(FormatString("IntPara '%1' out of range (must be < %2)")
					.arg(name).arg(val.value), FUNC_ID);
			}
			break;
	}
}


}  // namespace IBK

