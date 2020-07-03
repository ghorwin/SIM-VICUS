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

#ifndef NANDRAD_SensorH
#define NANDRAD_SensorH

#include <string>
#include <map>
#include <vector>

class TiXmlElement;

namespace NANDRAD {

/*!	\brief Declaration for class Sensor

	A sensor places a measured component for later referencing. It may be
	parametrized with string, constant parameters or flags. Use sensors inside
	the description block of the building component that offers the quantity.
*/
class Sensor {
public:

	/*! Default constructor. */
	Sensor();

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Reads the data from the xml element.
		Throws an IBK::Exception if a syntax error occurs.
	*/
	void readXML(const TiXmlElement * element);
	/*! Appends the element to the parent xml element.
		Throws an IBK::Exception in case of invalid data.
	*/
	void writeXML(TiXmlElement * parent, bool detailedOutput) const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID-number of the sensor.*/
	unsigned int						m_id;
	/*! Name of the measured quantity */
	std::string							m_quantity;
};

} // namespace NANDRAD

#endif // SensorH
