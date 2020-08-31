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

#ifndef NANDRAD_ScheduleH
#define NANDRAD_ScheduleH

#include <string>
#include <vector>

#include <IBK_Time.h>
#include <IBK_Unit.h>

#include "NANDRAD_DailyCycle.h"
#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*!	\brief Declaration for class Schedule

	Schedules are defined for a specific day type: week day,
	week end, holiday or all days. They contain one or more daily cycles
	that specify the distribution for different scheduled quantities over
	a period of one day.
	Holidays and week end days are defined in Schedules.
*/
class Schedule {
public:

	/*! Different day types a schedule can be defined for. */
	enum type_t {
		ST_ALLDAYS,		// Keyword: AllDays		'All days (Weekend days and Weekdays).'
		ST_WEEKDAY,		// Keyword: WeekDay		'Weekday schedule.'
		ST_WEEKEND,		// Keyword: WeekEnd		'Weekend schedule.'
		ST_HOLIDAY,		// Keyword: Holiday		'Holiday schedule.'
		ST_MONDAY,		// Keyword: Monday		'Special Weekday schedule: Monday.'
		ST_TUESDAY,		// Keyword: Tuesday		'Special Weekday schedule: Tuesday.'
		ST_WEDNESDAY,	// Keyword: Wednesday	'Special Weekday schedule: Wednesday.'
		ST_THURSDAY,	// Keyword: Thursday	'Special Weekday schedule: Thursday.'
		ST_FRIDAY,		// Keyword: Friday		'Special Weekday schedule: Friday.'
		ST_SATURDAY,	// Keyword: Saturday	'Special Weekday schedule: Saturday.'
		ST_SUNDAY,		// Keyword: Sunday		'Special Weekday schedule: Sunday.'
		NUM_ST
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE
	NANDRAD_COMP(Schedule)

	/*! Prepares calculation by initializing daily cycles and by collecting names of all scheduled parameters. */
	void prepareCalculation();

	/*! Returns true, if given day is inside the start and end date of the schedule. */
	bool containsDay(unsigned int dayOfYear) const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Type of day this schedule is defined for. */
	type_t					m_type = NUM_ST;								// XML:A:required

	/*! Start day of the year for the schedule, if not given, defaults to 0 = 1.1. */
	unsigned int			m_startDayOfTheYear = 0;						// XML:E

	/*! End day for the schedule (includes the entire day), if not given, defaults 364 = 31.12. */
	unsigned int			m_endDayOfTheYear = 364;						// XML:E

	/*! List of daily cycles that are used on day type specified above.
		These cycles define different quantities/control parameters etc.
	*/
	std::vector<DailyCycle> m_dailyCycles;									// XML:E


	// *** solver runtime variables only (not written to file) ***

	/*! All value names provided by all of the daily cycles.
		This set is composed during prepareCalculation() and primarily used to quickly check
		if this schedule provides a required parameter at all.
	*/
	std::set<std::string>	m_valueNames;
};

} // namespace NANDRAD

#endif // NANDRAD_ScheduleH
