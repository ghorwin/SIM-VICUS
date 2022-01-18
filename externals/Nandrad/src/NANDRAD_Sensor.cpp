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

#include "NANDRAD_Sensor.h"

namespace NANDRAD {

bool Sensor::operator!=(const Sensor & other) const {
	if (m_id != other.m_id)				return true;
	if (m_quantity != other.m_quantity)	return true;
	return false;
}


void Sensor::checkParameters() const {
	m_inclination.checkedValue("Inclination", "Deg", "Deg",
							   0, true,
							   180, true,
							   "Inclination must be between 0 and 180 Deg.");

	m_orientation.checkedValue("Orientation","Deg", "Deg",
							   0, true,
							   360, true,
							   "Inclination must be between 0 and 360 Deg.");
}

void Sensor::createSensor(unsigned int id, double orientationDeg, double inclinationDeg){
	m_id = id;
	m_orientation = IBK::Parameter("Orientation", orientationDeg, IBK::Unit("Deg"));
	m_inclination = IBK::Parameter("Inclination", inclinationDeg, IBK::Unit("Deg"));
	checkParameters();
}

} // namespace NANDRAD

