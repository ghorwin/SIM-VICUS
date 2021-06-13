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

#include "VICUS_ZoneControlThermostat.h"

#include "VICUS_KeywordList.h"

namespace VICUS {


bool ZoneControlThermostat::isValid(const Database<Schedule> & scheduleDB) const {
	if(m_id == INVALID_ID)
		return false;

	if (m_controlValue == NUM_CV ||
			m_controllerType == NUM_CT)
		return false;


	// if both setpoint schedules are undefined, the definition is incomplete (one is required)
	if (m_coolingSetpointScheduleId == INVALID_ID && m_heatingSetpointScheduleId == INVALID_ID)
		return false;

	try {
		if (m_heatingSetpointScheduleId != INVALID_ID) {
			m_para[P_Tolerance].checkedValue(KeywordList::Keyword("ZoneControlThermostat::para_t", P_Tolerance),
								 "K", "K", 0, true, 5, true, nullptr);
			// check if schedule ID is existing and valid
			const Schedule * sched = scheduleDB[m_heatingSetpointScheduleId];
			if (sched == nullptr)
				return false;
			if (!sched->isValid())
				return false;
		}
	}
	catch (...) {
		return false;
	}

	return true;
}


AbstractDBElement::ComparisonResult ZoneControlThermostat::equal(const AbstractDBElement *other) const {
	const ZoneControlThermostat * otherCtrl = dynamic_cast<const ZoneControlThermostat*>(other);
	if (otherCtrl == nullptr)
		return Different;

	//first check critical data

	//check parameters
	for(unsigned int i=0; i<NUM_P; ++i){
		if(m_para[i] != otherCtrl->m_para[i])
			return Different;
	}
	if(m_heatingSetpointScheduleId != otherCtrl->m_heatingSetpointScheduleId ||
			m_coolingSetpointScheduleId != otherCtrl->m_coolingSetpointScheduleId ||
			m_controlValue != otherCtrl->m_controlValue ||
			m_controllerType != otherCtrl->m_controllerType)
		return Different;

	//check meta data

	if(m_displayName != otherCtrl->m_displayName ||
			m_color != otherCtrl->m_color ||
			m_dataSource != otherCtrl->m_dataSource ||
			m_notes != otherCtrl->m_notes)
		return OnlyMetaDataDiffers;

	return Equal;
}


} // namespace VICUS
