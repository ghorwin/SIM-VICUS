/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "VICUS_InterfaceHeatConduction.h"
#include "VICUS_Project.h"

#include "VICUS_KeywordList.h"

namespace VICUS {

bool InterfaceHeatConduction::operator!=(const InterfaceHeatConduction & other) const {

	if (m_otherZoneType != other.m_otherZoneType)
		return true;
	for (int i=0; i<NUM_P; ++i)
		if (m_para[i] != other.m_para[i]) return true;
	if (m_idSchedule != other.m_idSchedule)
		return true;

	return false;
}


bool InterfaceHeatConduction::isValid(const Database<Schedule> &scheduleDB) const {
	// TODO : Implement isValid
	if(m_otherZoneType == VICUS::InterfaceHeatConduction::OZ_Scheduled) {
		if(m_idSchedule == INVALID_ID)
			return false;

		const Schedule * zoneSched = scheduleDB[m_idSchedule];
		if (zoneSched == nullptr)
			return false;
		if (!zoneSched->isValid())
			return false;
	}

	return true;
}


} // namespace VICUS

