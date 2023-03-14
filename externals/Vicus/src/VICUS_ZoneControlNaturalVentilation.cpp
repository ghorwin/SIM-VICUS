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

#include "VICUS_ZoneControlNaturalVentilation.h"
#include "VICUS_KeywordList.h"

namespace VICUS {


bool ZoneControlNaturalVentilation::isValid(const Database<Schedule> &scheduleDB) const
{
	// we have to check if the object is valid
	std::string err;
	// is id valid?
	if ( m_id == INVALID_ID )
		return false;

	for (int i = 0; i < NUM_P; ++i) {
		// is a schedule ID set?
		if (m_idSchedules[i] != INVALID_ID) {
			// ensure, that no schedule for comfort air change rate was set
			if(i == P_MaximumAirChangeRateComfort)
				return false;
			// ensure, that no schedule for wind speed was set
			if(i == P_WindSpeedMax)
				return false;

			// check if schedule ID is existing and valid
			const Schedule * sched = scheduleDB[m_idSchedules[i]];

			if (sched == nullptr)
				return false;

			if (!sched->isValid(err, true))
				return false;
			// either scheudle or parameter are requested
			continue;
		}

		try {
			switch (i) {
			case P_TemperatureAirMax:
			case P_TemperatureAirMin:
				// check whether a parameter with the correct unit has been set
				m_para[i].checkedValue(VICUS::KeywordList::Keyword("ZoneControlNaturalVentilation::para_t", i),
													 "C", "C", -100, true, 100, true, nullptr);
			break;
			case P_MaximumAirChangeRateComfort:
				// check whether a parameter with the correct unit has been set
				m_para[i].checkedValue(VICUS::KeywordList::Keyword("ZoneControlNaturalVentilation::para_t", i),
													 "1/h", "1/h", 0, false, 30, false, nullptr);
			break;
			case P_WindSpeedMax:
				// check whether a parameter with the correct unit has been set
				m_para[i].checkedValue(VICUS::KeywordList::Keyword("ZoneControlNaturalVentilation::para_t", i),
													 "m/s", "m/s", 0, true, 40, true, nullptr);
			break;
			}
		}  catch (...) {
			return false;
		}

	}
	if(m_para[P_TemperatureAirMax].value <= m_para[P_TemperatureAirMin].value)
		return false;

	return true;

}

AbstractDBElement::ComparisonResult ZoneControlNaturalVentilation::equal(const AbstractDBElement *other) const {
	const ZoneControlNaturalVentilation * otherVent = dynamic_cast<const ZoneControlNaturalVentilation*>(other);
	if (otherVent == nullptr)
		return Different;

	//first check critical data

	//check parameters
	for(unsigned int i=0; i<NUM_P; ++i){
		if(m_para[i] != otherVent->m_para[i])
			return Different;
	}

	for(unsigned int i=0; i<NUM_P; ++i)
		if(m_idSchedules[i] != otherVent->m_idSchedules[i])
			return Different;

	//check meta data

	if(m_displayName != otherVent->m_displayName ||
			m_color != otherVent->m_color ||
			m_dataSource != otherVent->m_dataSource ||
			m_notes != otherVent->m_notes)
		return OnlyMetaDataDiffers;

	return Equal;
}


}
