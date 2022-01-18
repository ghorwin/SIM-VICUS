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

#ifndef NANDRAD_SensorH
#define NANDRAD_SensorH

#include <IBK_Parameter.h>

#include "NANDRAD_Constants.h"
#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*!	A sensor yields a measured component for use by other models/for output purposes.

	The quantity can be used for further classify the type of sensor. In some cases, like
	in case of radiation sensors, the quantity can be omitted since placing the sensor
	will automatically generate 4 quantities (direct, diffuse, global radiations and incidence angle).
	The use of the quantity depends on the model implementing sensor support.
*/
class Sensor {
	NANDRAD_READWRITE_PRIVATE
public:
	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE_IFNOTEMPTY(Sensor)
	NANDRAD_COMP(Sensor)
	NANDRAD_COMPARE_WITH_ID

	/*! Checks for valid parameters (value ranges). */
	void checkParameters() const;

	/*! Quick create sensor helper function.
		automatic calls checkParameters()
		\param id -> sensor id
		\param orientationDeg -> orientation in Deg, 0 - north, 90 - east, ...
		\param inclinationDeg -> inclination in Deg, 0 - roof, 90 - wall, 180 - floor
	*/
	void createSensor(unsigned int id, double orientationDeg, double inclinationDeg);

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID-number of the sensor.*/
	unsigned int						m_id = NANDRAD::INVALID_ID;		// XML:A:required
	/*! Name of the measured quantity */
	std::string							m_quantity;						// XML:E

	/*! Orientation of the radiation load sensor [Deg] (see also ConstructionInstance). */
	IBK::Parameter						m_orientation;					// XML:E
	/*! Inclination of the radiation load sensor [Deg] (see also ConstructionInstance). */
	IBK::Parameter						m_inclination;					// XML:E
};

} // namespace NANDRAD

#endif // NANDRAD_SensorH
