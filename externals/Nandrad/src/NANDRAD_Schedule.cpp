/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#include "NANDRAD_Schedule.h"
#include "NANDRAD_KeywordList.h"
#include "NANDRAD_Constants.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <IBK_messages.h>

#include <tinyxml.h>

namespace NANDRAD {


bool Schedule::operator!=(const Schedule & other) const {
	if (m_type != other.m_type) return true;
	if (m_dailyCycles != other.m_dailyCycles) return true;

	return false;
}


void Schedule::prepareCalculation() {
	FUNCID(Schedule::prepareCalculation);
	// loop over all daily cycles and initialize them
	for (unsigned int i=0; i<m_dailyCycles.size(); ++i) {
		NANDRAD::DailyCycle & dc = m_dailyCycles[i];
		try {
			dc.prepareCalculation();
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error initializing DailyCycle #%1.").arg(i), FUNC_ID);
		}
		m_valueNames.insert(dc.m_valueNames.begin(), dc.m_valueNames.end());
	}
}


bool Schedule::containsDay(unsigned int dayOfYear) const {
	return dayOfYear >= m_startDayOfTheYear && dayOfYear <= m_endDayOfTheYear;
}


unsigned int Schedule::priority() const {
	FUNCID(Schedule::priority);
	switch (m_type) {
		case ST_ALLDAYS :		return 3;
		case ST_WEEKDAY :		return 2;
		case ST_WEEKEND :		return 2;
		case ST_HOLIDAY :		return 0;
		case ST_MONDAY :		return 1;
		case ST_TUESDAY :		return 1;
		case ST_WEDNESDAY :		return 1;
		case ST_THURSDAY :		return 1;
		case ST_FRIDAY :		return 1;
		case ST_SATURDAY :		return 1;
		case ST_SUNDAY :		return 1;
		default :
			throw IBK::Exception("Accessing undefined/invalid priority.", FUNC_ID);
	}
}


} // namespace NANDRAD

