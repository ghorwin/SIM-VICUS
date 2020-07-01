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

#ifdef _MSC_VER
	typedef __int64 int64_t;
	typedef unsigned __int64 uint64_t;
	typedef unsigned __int32 uint32_t;
	typedef unsigned __int16 uint16_t;
#else
	#include <stdint.h>
#endif

#include <iostream>
#include <iomanip>
#include <iterator>
#include <sstream>

#include "IBK_InputOutput.h"
#include "IBK_UnitVector.h"
#include "IBK_UnitList.h"
#include "IBK_messages.h"
#include "IBK_FormatString.h"
#include "IBK_math.h"
#include "IBK_Exception.h"

namespace IBK {

void UnitVector::read(const std::string& str, bool convertToSI) {

	std::stringstream strm(str);
	m_data.clear();

	// read the data
	std::copy(std::istream_iterator<double>(strm),
			  std::istream_iterator<double>(),
			  std::back_inserter(m_data) );
	if (m_data.empty())
		throw IBK::Exception( "Error reading vector data. String was empty or did not contain "
							  "any values (started with string or similar)!","[UnitVector::read]");

	// reset stream state to read unit
	strm.clear();
	std::string ustr;
	strm >> ustr;

	// No unit found...
	if (ustr.empty())
		return;

	if( convertToSI) {
		try {  m_unit = Unit(ustr);  }
		catch (...) {
			throw IBK::Exception( IBK::FormatString("Invalid vector unit '%1'!").arg(ustr),
								  "[UnitVector::read]" );
		}
		convert( Unit(m_unit.base_id()) );   // convert data to base unit
	}
	else {
		m_unit.set(0);
	}
}
// ---------------------------------------------------------------------------

void UnitVector::write(std::ostream& out, unsigned int indent, bool includeUnit) const {
	try {
		// create temporary vector
		UnitVector tmp(*this);
		// convert from base unit to IO-unit
		Unit target = m_unit;
		tmp.m_unit = Unit(m_unit.base_id());
		tmp.convert( target );
		// write data
		out << std::string(indent,' ');
		std::copy(tmp.m_data.begin(), tmp.m_data.end(), std::ostream_iterator<double>(out, " "));
		if (includeUnit)
			out << m_unit.name();
		// this was commented out by stvo since it is used by XML write functions
		// and produces errors we need discussion here!
//		out << '\n';
	}
	catch (std::exception& ex) {
		IBK::IBK_Message(ex.what(), MSG_ERROR, "[UnitVector::write]", 1);
		throw IBK::Exception("Error writing unit vector!","[UnitVector::write]");
	}
}

// ---------------------------------------------------------------------------

void UnitVector::writeBinary(std::ostream& out) const {

	IBK::write_string_binary( out, m_name);
	size_t size = m_data.size();
	out.write(reinterpret_cast<const char *>(&size), sizeof(size_t));
	for (	std::vector< double >::const_iterator it = m_data.begin(), end = m_data.end();
			it != end;
			++it)
	{
		out.write(reinterpret_cast<const char *>(&(*it)), sizeof(double));
	}
}

// ---------------------------------------------------------------------------

void UnitVector::readBinary( std::istream& in ){

	IBK::read_string_binary(in, m_name, 10000);
	size_t size;
	in.read(reinterpret_cast<char *>(&size), sizeof(size_t));

	m_data.resize(size);

	double value;
	for (size_t i = 0; i < size; ++i ){
		in.read(reinterpret_cast<char *>(&value), sizeof(double));
		m_data[i] = value;
	}
}

// ---------------------------------------------------------------------------

std::string UnitVector::toString(bool includeUnit) const {
	std::stringstream strm;
	write(strm,0,includeUnit);
	return strm.str();
}
// ---------------------------------------------------------------------------

void UnitVector::set(unsigned int n, double value, const std::string& ustr) {
	m_data.resize(n);
	std::fill(m_data.begin(), m_data.end(), value);
	m_unit.set(ustr);
}
// ---------------------------------------------------------------------------

void UnitVector::clear() {
	m_data.clear();
	m_unit.set(0);
}
// ---------------------------------------------------------------------------

bool UnitVector::operator!=(const UnitVector & uvec) const {
	if (m_unit != uvec.m_unit) return true;
	if (m_data.size() != uvec.m_data.size()) return true;
	if (m_data != uvec.m_data) return true;
	return false;
}
// ---------------------------------------------------------------------------

void UnitVector::convert(Unit target_unit) {
	if (m_unit == target_unit)  return;
	double fact;
	unsigned int op;
	m_unit.relate_to(target_unit, fact, op);
	if (op==UnitList::OP_ADD) {
		std::vector<double>::iterator endIt = m_data.end();
		for (std::vector<double>::iterator it=m_data.begin(); it!=endIt; ++it)
			*it += fact;
	}
	else if (op==UnitList::OP_MUL) {
		std::vector<double>::iterator endIt = m_data.end();
		for (std::vector<double>::iterator it=m_data.begin(); it!=endIt; ++it)
			*it *= fact;
	}
	else if (op==UnitList::OP_SPECIAL) {
		// TODO : this needs a major performance tweak!!!
		std::vector<double>::iterator endIt = m_data.end();
		for (std::vector<double>::iterator it=m_data.begin(); it!=endIt; ++it)
			convert_special(target_unit, *it);
	}
	m_unit = target_unit;
}
// ---------------------------------------------------------------------------

void UnitVector::convert_special(Unit target_unit, double & val) {
	const UnitData* src( UnitList::instance().retrieve(m_unit.id()) );
	const UnitData* target( UnitList::instance().retrieve(target_unit.id()) );
	const UnitData* base( UnitList::instance().retrieve(src->base_id()) );

	double base_val = val;
	if (m_unit == Unit("log(kg/m3)")) {
		if (val <= -50) base_val = 1e-50;
		else base_val = IBK::f_pow10(val);         // from log(kg/m3) -> kg/m3
	}
	else if (m_unit == Unit("log(g/m3)")) {
		if (val <= -50) base_val = 1e-50;
		else base_val = IBK::f_pow10(val)/1e+3;    // from log(g/m3) -> kg/m3
	}
	else if (m_unit == Unit("log(mg/m3)")) {
		if (val <= -50) base_val = 1e-50;
		else base_val = IBK::f_pow10(val)/1e+6;    // from log(mg/m3) -> kg/m3
	}
	else if (m_unit == Unit("log(µg/m3)")) {
		if (val <= -50) base_val = 1e-50;
		else base_val = IBK::f_pow10(val)/1e+9;    // from log(µg/m3) -> kg/m3
	}
	else if (m_unit == Unit("sqrt(s)")) {
		base_val = val*val;    // from sqrt(s) -> s
	}
	else if (m_unit == Unit("sqrt(h)")) {
		base_val = val*val*3600;    // from sqrt(h) -> s
	}
	else {
		 UnitList::instance().convert(src, base, base_val);
	}

	// base_val has the value in the base unit

	if (target_unit == Unit("log(kg/m3)")) {
		if (base_val <= 1e-50) val = -50;
		else val = IBK::f_log10(base_val);         // from kg/m3 -> log(kg/m3)
	}
	else if (target_unit == Unit("log(g/m3)")) {
		if (base_val <= 1e-50) val = -50;
		else val = IBK::f_log10(base_val*1e+3);    // from kg/m3 -> log(g/m3)
	}
	else if (target_unit == Unit("log(mg/m3)")) {
		if (base_val <= 1e-50) val = -50;
		else val = IBK::f_log10(base_val*1e+6);    // from kg/m3 -> log(mg/m3)
	}
	else if (target_unit == Unit("log(µg/m3)")) { // \todo micro micro gramm
		if (base_val <= 1e-50) val = -50;
		else val = IBK::f_log10(base_val*1e+9);    // from kg/m3 -> log(µg/m3)
	}
	else if (target_unit == Unit("sqrt(s)")) {
		if (base_val < 0)
			throw IBK::Exception("Conversion error (square-root of negative number)!", "[UnitVector::convert_special]");
		val = std::sqrt(base_val);    // from s -> sqrt(s)
	}
	else if (target_unit == Unit("sqrt(h)")) {
		if (base_val < 0)
			throw IBK::Exception("Conversion error (square-root of negative number)!", "[UnitVector::convert_special]");
		val = std::sqrt(base_val/3600);    // from s -> sqrt(h)
	}
	else {
		val = base_val;
		UnitList::instance().convert(base, target, val);
	}

}
// ---------------------------------------------------------------------------

#ifdef IBK_DEBUG
double & UnitVector::operator[] (unsigned int i) {
	if (i > m_data.size())
		throw IBK::Exception(FormatString("Index %1 out of range [%2 .. %3].")
			.arg(i).arg(0).arg(static_cast<unsigned int>(size()-1)),
			"[IBK::UnitVector::operator[] ]");
	return m_data[i];
}
// ---------------------------------------------------------------------------

const double & UnitVector::operator[] (unsigned int i) const {
	if (i > m_data.size())
		throw IBK::Exception(FormatString("Index %1 out of range [%2 .. %3].")
			.arg(i).arg(0).arg(static_cast<unsigned int>(size()-1)),
			"[IBK::UnitVector::operator[] ]");
	return m_data[i];
}
// ---------------------------------------------------------------------------

#endif // IBK_DEBUG

}  // namespace IBK
