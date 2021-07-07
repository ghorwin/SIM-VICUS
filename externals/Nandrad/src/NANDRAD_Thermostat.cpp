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

#include "NANDRAD_Thermostat.h"

#include "NANDRAD_KeywordList.h"

namespace NANDRAD {

void Thermostat::checkParameters() const {

	switch (m_controllerType) {
		case NANDRAD::Thermostat::NUM_CT:
		case NANDRAD::Thermostat::CT_Analog:
			// controller needs temperature tolerance
			m_para[P_TemperatureTolerance].checkedValue("TemperatureTolerance", "K", "K", 0, false, 10, true,
														"Temperature tolerance should be 0 K < tolerance < 10 K");
		break;

		case NANDRAD::Thermostat::CT_Digital:
			m_para[P_TemperatureBand].checkedValue("TemperatureBand", "K", "K", 0, false, 10, true,
														"Temperature band should be 0 K < band < 10 K");

		break;
	}

}

bool Thermostat::equal(const Thermostat &other) const {
	//check parameters
	for(unsigned int i=0; i<NUM_P; ++i){
		if(m_para[i] != other.m_para[i])
			return false;
	}

	if(m_modelType != other.m_modelType ||
			m_controllerType != other.m_controllerType ||
			m_temperatureType != other.m_temperatureType ||
			m_referenceZoneId != other.m_referenceZoneId)
		return false;

	return true;
}


} // namespace NANDRAD

