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

#ifndef NANDRAD_SchedulesH
#define NANDRAD_SchedulesH

#include <string>
#include <vector>

#include <IBK_Flag.h>

#include "NANDRAD_Schedule.h"
#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*!	Schedules define purely time-dependent properties.

	Scheduled quantities are associated with models via object lists. The object list
	names are the keys to the maps m_scheduleGroups and m_annualSchedules.
*/
class Schedules {
public:

	/*! Day ids, also used to define "a weekend definition". */
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

	/*! Flags controlling schedule evaluation. */
	enum flag_t {
		/*! If enabled, schedules are treated as annually repeating schedules. */
		F_EnableCyclicSchedules,	// Keyword: EnableCyclicSchedules			'If enabled, schedules are treated as annually repeating schedules.'
		NUM_F
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Init default values (called before readXML()).
		\note These values will be overwritten in readXML() when the respective property is set
			  in the project file.
	*/
	void initDefaults();

	NANDRAD_READWRITE
	NANDRAD_COMP(Schedules)


	/*! Checks given parameters and initializes all linear splines given as tsv-files. */
	void checkParameters(const std::map<std::string, IBK::Path> &placeholders);

	/*! This function does all the work in the actual schedule-by-daily-cycle implementation.
		It constructs a linear spline for cyclic annual use by processing all days of the year,
		looking up the respective parameter and combining day cycle values.

		\param objectListName Name of schedule group (i.e. name of corresponding object list)
		\param parameterName Name of scheduled parameter
		\param spline Here the spline data will be stored, values are already converted to the base SI unit.
		\param interpolationType Here the interpolation type set for the daily cycle will be set.
	*/
	void generateLinearSpline(const std::string & objectListName, const std::string & parameterName,
							  IBK::LinearSpline & spline,
							  DailyCycle::interpolation_t & interpolationType
							  ) const;


	/*! Compares two schedule groups (period) by schedule content, hereby ignoring the name of the schedule group. */
	static bool equalSchedules(const std::vector<Schedule> & first, const std::vector<Schedule> & second);

	/*! Compares two schedule groups (annual) by schedule content, hereby ignoring the name of the schedule group. */
	static bool equalAnnualSchedules(const std::vector<NANDRAD::LinearSplineParameter> & first, const std::vector<NANDRAD::LinearSplineParameter> & second);


	// *** PUBLIC MEMBER VARIABLES ***

	/*! List of holiday days, stored in "day of the year", not including leap days. */
	std::set< unsigned int>													m_holidays;

	/*! Weekend days. */
	std::set< day_t >														m_weekEndDays;

	/*! The daytype of January 1st (offset of day of the week (0-Mon, ...6-Sun)) of the start year,
		defaults to Monday.
	*/
	day_t																	m_firstDayOfYear = SD_MONDAY;

	/*! List of flags. */
	IBK::Flag																m_flags[NUM_F];

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
