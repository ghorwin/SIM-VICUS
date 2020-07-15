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

#ifndef NANDRAD_SpaceTypeH
#define NANDRAD_SpaceTypeH

class TiXmlElement;

#include <string>
#include <IBK_Parameter.h>

namespace NANDRAD {

/*!	\brief Declaration for class SpaceType
	A space type defines all properties of a given space/zone type.
*/
class SpaceType {
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
	bool operator!=(const SpaceType & other) const;
	/*! Compares this instance with another by value and returns true if they are the same. */
	bool operator==(const SpaceType & other) const { return ! operator!=(other); }

	// *** PUBLIC MEMBER VARIABLES ***

	/*! ID-name of this space type. */
	std::string								m_name;

};

} // namespace NANDRAD

#endif // NANDRAD_SpaceTypeH
