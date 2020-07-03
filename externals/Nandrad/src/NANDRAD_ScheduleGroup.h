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

#ifndef NANDRAD_ScheduleGroupH
#define NANDRAD_ScheduleGroupH

#include <string>

#include "NANDRAD_Schedule.h"

class TiXmlElement;

namespace NANDRAD {

/*!	\brief Declaration for class ScheduleGroup

	A schedule group contains a list of schedules for a specifific space type.
	They may optionally be restricted to a time period of the year from
	start date to end date. In that case define a second schedule for
	the same space type. Ensure that all scheduled quantities are completely defined
	over the whole year.
*/
class ScheduleGroup {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Reads the data from the xml element.
		Throws an IBK::Exception if a syntax error occurs.
	*/
	void readXML(const TiXmlElement * element);

	/*! Appends the element to the parent xml element.
		Throws an IBK::Exception in case of invalid data.
	*/
	void writeXML(TiXmlElement * parent, bool detailedOutput) const;

	/*! Compares this instance with another by value and returns true if they differ. */
	bool operator!=(const ScheduleGroup & other) const;

	/*! Compares this instance with another by value and returns true if they are the same. */
	bool operator==(const ScheduleGroup & other) const { return ! operator!=(other); }

	// *** PUBLIC MEMBER VARIABLES ***

	typedef std::map<Schedule::type_t, Schedule>	ScheduleMap;

	/*! Start date for the schedule. */
	IBK::Time							m_startDate;

	/*! End date for the schedule. */
	IBK::Time							m_endDate;

	/*! Map with SpaceTypeGroups and their schedules.
		The keys of the map are the SpaceType ID-names. The values are maps
		of Schedules defined within the SpaceTypeGroup.
	*/
	std::map<std::string, ScheduleMap>	m_spaceTypeGroups;

private:
	/*! Reads the data from the xml element.
		Throws an IBK::Exception if a syntax error occurs.
	*/
	void readSpaceTypeGroupXML(const TiXmlElement * element);

	/*! Appends the element to the parent xml element.
		Throws an IBK::Exception in case of invalid data.
	*/
	void writeSpaceTypeGroupXML(TiXmlElement * parent, bool detailedOutput) const;
};

} // namespace NANDRAD

#endif // ScheduleGroupH
