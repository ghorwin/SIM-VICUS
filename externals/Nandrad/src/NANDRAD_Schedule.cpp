/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

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
	if (m_startDayOfTheYear != other.m_startDayOfTheYear) return true;
	if (m_endDayOfTheYear != other.m_endDayOfTheYear) return true;

	return false;
}


void Schedule::prepareCalculation() {
	FUNCID(Schedule::prepareCalculation);

	// For holiday it is not allowed to have a periode definition
	if(m_type == ST_HOLIDAY && (m_startDayOfTheYear != 0 || m_endDayOfTheYear != 364))
		throw IBK::Exception(IBK::FormatString("For a schedule  with type 'Holiday' it is not allowed to define a period."), FUNC_ID);

	// loop over all daily cycles and initialize them
	for (unsigned int i=0; i<m_dailyCycles.size(); ++i) {
		NANDRAD::DailyCycle & dc = m_dailyCycles[i];
		try {
			dc.prepareCalculation();
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error initializing DailyCycle #%1.").arg(i), FUNC_ID);
		}
		for (const DailyCycle::valueData_t & paraData : dc.m_valueData) {
			if (m_parameters.find(paraData.m_name) != m_parameters.end())
				throw IBK::Exception(IBK::FormatString("Multiple DailyCycle blocks define the same parameter '%1'. This is not allowed!")
									 .arg(paraData.m_name), FUNC_ID);
			m_parameters[paraData.m_name] = &dc; // persistent pointer guaranteed - we do not modifiy the m_dailyCycles vector anylonger
		}
	}
}

} // namespace NANDRAD

