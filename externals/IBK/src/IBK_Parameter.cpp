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

#include "IBK_Parameter.h"
#include "IBK_UnitList.h"
#include "IBK_Exception.h"
#include "IBK_StringUtils.h"

namespace IBK {

Parameter::Parameter(const std::string& str, double val, Unit unit)
	: name(str), value(val), IO_unit(unit)
{
	// only conversion if unit is set to a valid unit
	if (unit.id() != 0)
		UnitList::instance().convert( IO_unit, Unit(IO_unit.base_id()), value);
}

Parameter::Parameter(const std::string& str, double val, const std::string& unit_str)
	: name(str), value(val)
{
	// The constructor of class Unit may throw an exception if unit_str is invalid,
	// but that's ok, no resources can be lost here. C++ takes care of it...
	IO_unit = Unit(unit_str);
	// only conversion if unit is set to a valid unit
	if (IO_unit.id() != 0)
		UnitList::instance().convert( IO_unit, Unit(IO_unit.base_id()), value);
}

bool Parameter::set(double val, Unit unit, std::string * errmsg) {
	value = val;
	IO_unit = unit;
	try {
		UnitList::instance().convert( IO_unit, Unit(IO_unit.base_id()), value);
	}
	catch (IBK::Exception & ex) {
		if (errmsg != NULL) *errmsg = ex.what();
		return false;
	}
	return true;
}

bool Parameter::set(const std::string& str, double val, Unit unit, std::string * errmsg) {
	name = str;
	return set(val, unit, errmsg);
}

void Parameter::set(const std::string& str, double val) {
	name = str;
	value = val;
	IO_unit.clear();
}

bool Parameter::set(const std::string& str, const std::string& valstr, std::string * errmsg) {
	std::stringstream lstrm(valstr);
	std::string ustr;
	lstrm >> value >> ustr;
	if (ustr.empty() || !lstrm) {
		if (errmsg != NULL) *errmsg = (FormatString("Error reading parameter from string '%1'.") .arg(valstr)).str();
		return false;
	}
	return set(str, value, ustr, errmsg);
}

double Parameter::get_value(Unit unit) const {
	double v=value; // copy parameter (don't change the actual base value)
	try {
		UnitList::instance().convert(Unit(IO_unit.base_id()), unit, v);
	}
	catch (IBK::Exception &  ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error converting unit in parameter '%1'.").arg(name), "[Parameter::get_value]");
	}
	return v;
}

double Parameter::get_value_or_default(Unit unit, double defaultValue) const {
	if (name.empty())
		return defaultValue;
	else
		return get_value(unit);
}

std::string Parameter::toString(bool without_name) const {
	std::stringstream strm;
	if (!without_name)
		strm << name << " = ";
	strm << get_value(IO_unit) << " " << IO_unit.name();
	return strm.str();
}

std::string Parameter::toString(const IBK::Unit & u, bool without_name) const {
	std::stringstream strm;
	if (!without_name)
		strm << name << " = ";
	strm << get_value(u) << " " << u.name();
	return strm.str();
}

void Parameter::read(std::istream& in, bool no_name) {
	std::string name_, unit_, equal_;
	double val_;
	if (no_name)
		in >> val_ >> unit_;            // don't read name
	else
		in >> name_ >> equal_ >> val_ >> unit_;   // read name, too
	if (!in)
		throw IBK::Exception( "Error reading parameter.", "[Parameter::read]");
	std::string errmsg;
	if (!set(val_, Unit(unit_), &errmsg))
		throw IBK::Exception(errmsg, "[Parameter::read]");
	if (!no_name)
		name = name_;        // assign read name
}

void Parameter::write(std::ostream& out, unsigned int indent, unsigned int paramWidth, bool writeName) const {
	double v=value;
	UnitList::instance().convert(Unit(IO_unit.base_id()), IO_unit, v);
	out << std::string(indent, ' ');
	if (name.size() && writeName) {
		if (paramWidth > name.size())
			out << std::setw(paramWidth) << std::left << name << " = ";
		else
			out << name << " = ";
	}
	out << v << " " << IO_unit.name() << std::endl;
}

void Parameter::readBinary(std::istream& in) {
	int size;
	in.read(reinterpret_cast<char*>(&size), sizeof(size));
	if (size < 0)
		throw IBK::Exception( "Error reading name size.", "[Parameter::readBinary]");
	if (size > 0) {
		std::vector<char> tempString(size+1, 0);
		in.read(&tempString[0], size);
		name = std::string(&tempString[0]);
	}
	in.read(reinterpret_cast<char*>(&value), sizeof(value));
	unsigned int unitId;
	in.read(reinterpret_cast<char*>(&unitId), sizeof(unitId));
	IO_unit.set(unitId);
}

void Parameter::writeBinary(std::ostream& out) const {
	int size = (int)name.size();
	out.write(reinterpret_cast<const char*>(&size), sizeof(size));
	if (size > 0)
		out.write(name.c_str(), size);
	out.write(reinterpret_cast<const char*>(&value), sizeof(value));
	unsigned int unitId = IO_unit.id();
	out.write(reinterpret_cast<const char*>(&unitId), sizeof(unitId));
}

void Parameter::clear() {
	name.clear();
	value=0;
	IO_unit=Unit(0);
}


void Parameter::checkIfValueIsLowerBound (	const IBK::Parameter& limit,
											bool isLessEqual ) const
{

	const char * const FUNC_ID = "[Parameter::checkIfValueIsLowerBound]";

	// check parameter
	if ( name.empty() || name != limit.name ) {
		std::string msg = limit.name + " is required";
		throw IBK::Exception(msg, FUNC_ID);
	}

	// check unit
	if (IO_unit.base_id() != limit.unit().base_id()) {
		std::string msg = "Invalid or unrecognized unit in parameter '" + limit.name +
						  "' (must be convertible into " + limit.unit().name() + ")";
		throw IBK::Exception(msg, FUNC_ID);
	}

	// test limits
	if (isLessEqual)
		test( limit, OP_LE);
	else
		test( limit, OP_LT);

}

void Parameter::checkIfValueIsUpperBound (	const IBK::Parameter& limit,
											bool isGreaterEqual ) const
{

	const char * const FUNC_ID = "[Parameter::checkIfValueIsUpperBound]";

	// check parameter
	if (name.empty() || name != limit.name ) {
		std::string msg = limit.name + " is required";
		throw IBK::Exception(msg, FUNC_ID);
	}

	// check unit
	if (IO_unit.base_id() != limit.unit().base_id()) {
		std::string msg = "Invalid or unrecognized unit in parameter '" + limit.name +
						  "' (must be convertible into " + limit.unit().name() + ")";
		throw IBK::Exception(msg, FUNC_ID);
	}

	// test limits
	if (isGreaterEqual)
		test( limit, OP_GE);
	else
		test( limit, OP_GT);

}

void Parameter::test( const IBK::Parameter& val, oper_t op ) const {

	static const char * const FUNC_ID = "[Parameter::test]";

	// convert and compare value
	switch (op) {
		case OP_GE :
			if ( !( value >= val.value ) ) {
				throw IBK::Exception(FormatString("Parameter '%1' out of range (must be >= %2 %3).")
									 .arg(name) .arg( val.get_value() ) .arg( val.IO_unit ), FUNC_ID);
			}
			break;

		case OP_GT :
			if (!(value > val.value)) {
				throw IBK::Exception(FormatString("Parameter '%1' out of range (must be > %2 %3).")
									 .arg(name) .arg( val.get_value() ) .arg( val.IO_unit ), FUNC_ID);
			}
			break;

		case OP_LE :
			if (!(value <= val.value)) {
				throw IBK::Exception( FormatString("Parameter '%1' out of range (must be <= %2 %3).")
									  .arg(name) .arg( val.get_value() ) .arg( val.IO_unit ), FUNC_ID);
			}
			break;

		case OP_LT :
			if (!(value < val.value)) {
				throw IBK::Exception(FormatString("Parameter '%1' out of range (must be < %2 %3).")
									 .arg(name) .arg( val.get_value() ) .arg( val.IO_unit ), FUNC_ID);
			}
			break;
	}
}



}  // namespace IBK

