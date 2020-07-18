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

#ifndef NANDRAD_SchedulesH
#define NANDRAD_SchedulesH

#include <string>
#include <vector>

#include "NANDRAD_Schedule.h"
#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*!	\brief Declaration for class Schedules

	A vector of all schedule groups.
	The default parameter section defines holidays and week end days
	for the current project and may be overwritten by a special schedule.
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

#if 0
	/*! Fill parameter with list of defined schedules.
		Parameter ID-names have the format SpaceType:IDName. Boolean defines whether this
		is a parameter defined via AnnualSchedule (true).
	*/
	void parameterList(std::map< std::string, bool > & parameterIDNames) const;

	/*! Returns a unit vector with date/time-value pairs for a selected parameter.
		Parameter must be one of the parameters returned from the parameterList() function.
	*/
	void parameterValues(const std::string & parameterIDName, NANDRAD::LinearSplineParameter & values) const;

	/*! Fill parameter with list of defined schedules.
		Boolean defines whether this is has a parameter defined via AnnualSchedule (true).
	*/
	void scheduleList(std::map< std::string, bool > & scheduleNames) const;
#endif
	// *** PUBLIC MEMBER VARIABLES ***


	/*! List of holiday dates. */
	std::set< IBK::Time >													m_holidays;

	/*! Weekend days. */
	std::set< day_t >														m_weekEndDays;

	/*! Key is object list name, value is vector of schedules. */
	std::map<std::string, std::vector<Schedule> >							m_scheduleGroups;
	/*! The LinearSplineParameter has a name, which corresponds to the
		object list that this scheduled parameter is for.
	*/
	std::vector<NANDRAD::LinearSplineParameter>								m_annualSchedules;


};

} // namespace NANDRAD

#endif // SchedulesH
