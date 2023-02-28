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

#include "VICUS_VentilationNatural.h"
#include "VICUS_KeywordList.h"

namespace VICUS {


bool VentilationNatural::isValid(const Database<Schedule> &scheduleDB) const {

	std::string err;
	if ( m_id == INVALID_ID )
		return false;

	if ( m_idSchedule == INVALID_ID )
		return false;
	else {
		// check if schedule ID is existing and valid
		const Schedule * sched = scheduleDB[m_idSchedule];
		if (sched == nullptr)
			return false;
		if (!sched->isValid(err, true))
			return false;
	}

	try {
		// check whether a parameter with the correct unit has been set
		m_para[P_AirChangeRate].checkedValue(VICUS::KeywordList::Keyword("VentilationNatural::para_t", P_AirChangeRate),
											 "1/h", "1/h", 0, true, 100, true, nullptr);
	}  catch (...) {
		return false;
	}

	return true;
}

AbstractDBElement::ComparisonResult VentilationNatural::equal(const AbstractDBElement *other) const {
	const VentilationNatural * otherVent = dynamic_cast<const VentilationNatural*>(other);
	if (otherVent == nullptr)
		return Different;

	//first check critical data

	//check parameters
	for(unsigned int i=0; i<NUM_P; ++i){
		if(m_para[i] != otherVent->m_para[i])
			return Different;
	}
	if(m_idSchedule != otherVent->m_idSchedule)
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
