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

#include "IBK_Unit.h"
#include "IBK_UnitData.h"
#include "IBK_UnitList.h"
#include "IBK_StringUtils.h"
#include "IBK_Exception.h"
#include "IBK_FormatString.h"

namespace IBK {

Unit::Unit(const std::string& name) {
	const UnitData* unit = UnitList::instance().retrieve(name);
	if (unit==NULL)  throw IBK::Exception( FormatString("Unit '%1' not found.").arg(name),"[Unit::Unit]");
	m_id=unit->id();
#ifdef IBK_ENABLE_UNIT_NAME
	m_name = unit->name();
#endif // IBK_ENABLE_UNIT_NAME
}

Unit::Unit(unsigned int unitid) {
	if (unitid>=UnitList::instance().size())
		throw IBK::Exception( FormatString("Unit index %1 out of range 0..%2.")
			.arg(unitid).arg((int)UnitList::instance().size()-1),"[Unit::Unit]");
	m_id=unitid;
#ifdef IBK_ENABLE_UNIT_NAME
	const UnitData* unit = UnitList::instance().retrieve(unitid);
	if (unit==NULL)  throw IBK::Exception( FormatString("Unit with ID '%1' not found.").arg(unitid),"[Unit::Unit]");
	m_name = unit->name();
#endif // IBK_ENABLE_UNIT_NAME
}

void Unit::set(unsigned int unitid) {
	if (unitid>=UnitList::instance().size())
		throw IBK::Exception( FormatString("Unit index %1 out of range 0..%2.")
			.arg(unitid).arg((int)UnitList::instance().size()-1),"[Unit::set]");
	m_id=unitid;
#ifdef IBK_ENABLE_UNIT_NAME
	m_name = UnitList::instance().retrieve(unitid)->name();
#endif // IBK_ENABLE_UNIT_NAME
}

void  Unit::set(const std::string& name) {
	const UnitData* unit = UnitList::instance().retrieve(name);
	if (unit==NULL)  throw IBK::Exception( FormatString("Unit '%1' not found.").arg(name),"[Unit::set]");
	m_id=unit->id();
#ifdef IBK_ENABLE_UNIT_NAME
	m_name = unit->name();
#endif // IBK_ENABLE_UNIT_NAME
}

const std::string& Unit::name() const {
	return UnitList::instance().retrieve(m_id)->name();
}

unsigned int Unit::base_id() const {
	return UnitList::instance().retrieve(m_id)->base_id();
}

void Unit::relate_to(unsigned int unitid, double& fact, unsigned int& op) {
	if (unitid==m_id) {
		fact=1;
		op=UnitList::OP_MUL;
		return;
	}
	const UnitData* from = UnitList::instance().retrieve( m_id );
	const UnitData* to = UnitList::instance().retrieve( unitid );
	UnitList::instance().relate_units(from, to, fact, op);
}

void Unit::relate_to(const std::string& name, double& fact, unsigned int& op) {
	unsigned int to_id = UnitList::instance().retrieve(name)->id();
	relate_to(to_id, fact, op);
}

void Unit::relate_to(Unit unit, double& fact, unsigned int& op) {
	relate_to(unit.m_id, fact, op);
}

std::istream& operator>>(std::istream& in, Unit& unit) {
	std::string uname;
	in >> uname;
	if (!in) return in;
	unit = Unit(uname);
	return in;
}

std::ostream& operator<<(std::ostream& out, const Unit& unit) {
	return out << unit.name();
}

}  // namespace IBK

