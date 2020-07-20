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

#include <IBK_Parameter.h>

#include "NANDRAD_Constants.h"
#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*!	\brief Declaration for class Sensor

	A sensor places a measured component for later referencing. It may be
	parametrized with string, constant parameters or flags. Use sensors inside
	the description block of the building component that offers the quantity.

	The quantity name must be recognized by the solver, see NANDRAD_MODEL::Loads::VectorValuedResults.
	For now it must be always equal to 'SWRadOnPlane'.
*/
class Sensor {
	NANDRAD_READWRITE_PRIVATE
public:
	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE_IFNOTEMPTY(Sensor)
	NANDRAD_COMP(Sensor)

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID-number of the sensor.*/
	unsigned int						m_id = NANDRAD::INVALID_ID;		// XML:A:required
	/*! Name of the measured quantity */
	std::string							m_quantity;						// XML:E

	/*! Orientation of the radiation load sensor. */
	IBK::Parameter						m_orientation;					// XML:E
	/*! Inclination of the radiation load sensor. */
	IBK::Parameter						m_inclination;					// XML:E
};

} // namespace NANDRAD

#endif // SensorH
