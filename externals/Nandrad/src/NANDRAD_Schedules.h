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

#ifndef NANDRAD_SchedulesH
#define NANDRAD_SchedulesH

#include <string>
#include <vector>

#include "NANDRAD_Schedule.h"
#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*!	Schedules define purely time-dependent properties.

	Scheduled quantities are associated with models via object lists. The object list
	names are the keys to the maps m_scheduleGroups and m_annualSchedules.

*/
class Schedules {
public:

	/*! Day ids used to define "a weekend". */
	enum day_t {
		SD_MONDAY,		// Keyword: Mon		'Monday.'
		SD_TUESDAY,		// Keyword: Tue		'Tuesday.'
		SD_WEDNESDAY,	// Keyword: Wed		'Wednesday.'
		SD_THURSDAY,	// Keyword: Thu		'Thursday.'
		SD_FRIDAY,		// Keyword: Fri		'Friday.'
		SD_SATURDAY,	// Keyword: Sat		'Saturday.'
		SD_SUNDAY,		// Keyword: Sun		'Sunday.'
		NUM_SD
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE
	NANDRAD_COMP(Schedules)

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Does basic consistency checks. */
	void checkParameters();


	/*! List of holiday dates. */
	std::set< IBK::Time >													m_holidays;

	/*! Weekend days. */
	std::set< day_t >														m_weekEndDays;

	/*! Key is object list name, value is vector of schedules. */
	std::map<std::string, std::vector<Schedule> >							m_scheduleGroups;
	/*! Key is object list name, value is vector of LinearSplineParameter.
		The LinearSplineParameter has a name, which corresponds to the
		quantity that this scheduled parameter is for.
	*/
	std::map<std::string, std::vector<NANDRAD::LinearSplineParameter> >		m_annualSchedules;
};

} // namespace NANDRAD

#endif // NANDRAD_SchedulesH
