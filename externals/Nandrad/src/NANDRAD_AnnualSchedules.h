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

#ifndef NANDRAD_AnnualSchedulesH
#define NANDRAD_AnnualSchedulesH

#include "NANDRAD_LinearSplineParameter.h"

class TiXmlElement;

namespace NANDRAD {

/*!	\brief Declaration for class AnnualSchedules.
	AnnualSchedules are enclosed within Schedules and comprise a map of
	AnnualSchedule objects sorted according to their SpaceType attributes.
*/
class AnnualSchedules {
public:

	/*! Reads the data from the xml element.
		Throws an IBK::Exception if a syntax error occurs.
	*/
	void readXML(const TiXmlElement * element);

	/*! Appends the element to the parent xml element.
		Throws an IBK::Exception in case of invalid data.
	*/
	void writeXML(TiXmlElement * parent) const;

	/*! Compares this instance with another by value and returns true if they differ. */
	bool operator!=(const AnnualSchedules & other) const;
	/*! Compares this instance with another by value and returns true if they are the same. */
	bool operator==(const AnnualSchedules & other) const { return ! operator!=(other); }


	// *** PUBLIC MEMBER VARIABLES ***

	/*! Type for map with annual schedules stored as linear spline parameters.
		Key of the map is the ID-Name of the physical property, value is the
		corresponding linear spline.
	*/
	typedef std::map<std::string, NANDRAD::LinearSplineParameter>	LinearSplineParameterMap;

	/*! Map with defined parameters for given SpaceTypeGroups.
		Key of the map is the SpaceType name, value is a map of
		parameters.
	*/
	std::map<std::string, LinearSplineParameterMap>		m_parameters;
};

} // namespace NANDRAD

#endif // AnnualSchedulesH
