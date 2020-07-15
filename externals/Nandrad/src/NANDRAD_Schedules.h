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
#include <set>

#include <IBK_UnitVector.h>

#include "NANDRAD_ScheduleGroup.h"
#include "NANDRAD_AnnualSchedules.h"

class TiXmlElement;

namespace NANDRAD {

/*!	\brief Declaration for class Schedules

	A vector of all schedule groups.
	The default parameter section defines holidays and week end days
	for the current project and may be overwritten by a special schedule.
*/
class Schedules {
public:
	// ***KEYWORDLIST-START***
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
	// ***KEYWORDLIST-END***
#if 0

	/*! Defines all defaults for schedule elements. */
	struct DefaultParameters {

		/*! Initializes WeekEndDays with defaults. */
		DefaultParameters();

		/*! Reads the data from the xml element.
			Throws an IBK::Exception if a syntax error occurs.
		*/
		void readXML(const TiXmlElement * element);

		/*! Appends the element to the parent xml element.
			Throws an IBK::Exception in case of invalid data.
		*/
		void writeXML(TiXmlElement * parent) const;

		/*! Compares this instance with another by value and returns true if they differ. */
		bool operator!=(const DefaultParameters & other) const;

		/*! Compares this instance with another by value and returns true if they are the same. */
		bool operator==(const DefaultParameters & other) const { return ! operator!=(other); }


		/*! List of holiday dates. */
		std::set< IBK::Time >		m_holidays;

		/*! Weekend days. */
		std::set< day_t >		m_weekEndDays;
	};

#endif

	/*! Reads the data from the xml element.
		Throws an IBK::Exception if a syntax error occurs.
	*/
	void readXML(const TiXmlElement * element);

	/*! Appends the element to the parent xml element.
		Throws an IBK::Exception in case of invalid data.
	*/
	void writeXML(TiXmlElement * parent) const;

	/*! Compares this instance with another by value and returns true if they differ. */
	bool operator!=(const Schedules & other) const;

	/*! Compares this instance with another by value and returns true if they are the same. */
	bool operator==(const Schedules & other) const { return ! operator!=(other); }


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

	// *** PUBLIC MEMBER VARIABLES ***


	/*! Definition of default parameters.*/
//	DefaultParameters					m_defaults;

	/*! Vector of schedule groups.
		First and only the first schedule group must not have a start and end date and
		last the whole year.
		The lookup of schedules is done bottom-to-top. Starting from
		last schedule group in vector, all schedule groups are tested
		for the requested time until a schedule group is found that
		covers this time point.
	*/
	std::vector<ScheduleGroup>			m_scheduleGroups;

	/*! Container for all annual schedules. */
	AnnualSchedules						m_annualSchedules;

private:

	/*! Writes everything below the "Schedules" tag.
		Throws an IBK::Exception in case of invalid data.
		\param element XML Element pointing to the root "Schedules" tag.
	*/
	void writeXML2(TiXmlElement * element) const;

};

} // namespace NANDRAD

#endif // SchedulesH
