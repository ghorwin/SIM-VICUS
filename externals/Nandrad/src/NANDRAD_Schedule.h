/*	The NANDRAD data model library.
Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
St. Vogelsang	<stefan.vogelsang -[at]- tu-dresden.de>
All rights reserved.

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
	Holidays and week end days can be specified using the default
	definition defined in Schedules::DefaultParameters. Or the are overwritten
	in the schedule definition.

*/
class Schedule {
public:

	/*! Don't change order in this array since several conversions rely on that. */
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

	/*! Populates set with quantities.
		A quantity is uniquely defined through its ID name and base unit.
		\param quantities Map with key-value - pairs of parameter ID names and corresponding base unit (value is always 0).
	*/
	void collectQuantities(const std::string & prefix, std::map<std::string, IBK::Unit> & quantities) const;

//	/*! Populates vector of IBK::Time elements from local schedule validity time definition.
//	*/
//	void collectValidDays(const unsigned int year,
//			const std::list<Date> &defaultHolidays,
//			const std::vector<day_t> &defaultWeekEndDays,
//			std::vector<IBK::Time> &time,
//			bool &allDays) const;

	// *** STATIC PUBLIC MEMBER FUNCTIONS ***

	/*! Returns the priority of an selected schedule.*/
	static int priority(type_t scheduleType);

	// *** PUBLIC MEMBER VARIABLES ***

	type_t					m_type = NUM_ST;								// XML:A:required

	/*! Start date for the schedule, if not given, defaults to 1.1. */
	IBK::Time				m_startDate;									// XMLS:E

	/*! End date for the schedule, if not given, defaults to 31.12. */
	IBK::Time				m_endDate;										// XMLS:E

	/*! List of daily cycles that are used on day type specified above.
		These cycles define different quantities/control parameters etc.
	*/
	std::vector<DailyCycle> m_dailyCycles;									// XML:E

	/*! Conversion function for special schedule time format. */
//	static void convertWeekDaysToIBKTime(const unsigned int year, const std::vector<day_t> &weekdays, std::vector<IBK::Time> &time);

	/*! Calculate the week day of a given date.
		\return Returns the index of a week day.
	*/
//	static unsigned int calcWeekDay(const int year, const NANDRAD::Date &date);

};

} // namespace NANDRAD

#endif // ScheduleH
