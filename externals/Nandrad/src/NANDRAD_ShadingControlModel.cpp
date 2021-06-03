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

#include "NANDRAD_ShadingControlModel.h"

#include "NANDRAD_ConstructionInstance.h"
#include "NANDRAD_EmbeddedObject.h"
#include "NANDRAD_Sensor.h"

#include "NANDRAD_KeywordList.h"

#include <algorithm>

namespace NANDRAD {

void ShadingControlModel::checkParameters(const std::vector<Sensor> &sensors,
										  const std::vector<ConstructionInstance> &conInstances)
{
	FUNCID(ShadingControlModel::checkParameters);

	// parameter unit and range
	m_para[P_MaxIntensity].checkedValue("MaxIntensity", "W/m2", "W/m2",
							   0, true,
							   std::numeric_limits<double>::max(), true,
							   "MaxIntensity must be > 0.");

	m_para[P_MinIntensity].checkedValue("MinIntensity", "W/m2", "W/m2",
							   0, true,
							   std::numeric_limits<double>::max(), true,
							   "MinIntensity must be > 0.");

	if(m_para[P_MaxIntensity].value <= m_para[P_MinIntensity].value) {
		throw IBK::Exception(IBK::FormatString("MaxIntensity must be greater than MinIntensity."), FUNC_ID);
	}

	// test if m_sensor is the ID of a sensor
	std::vector<Sensor>::const_iterator sit =
			std::find(sensors.begin(), sensors.end(), m_sensorId);
	if (sit != sensors.end()) {
		m_sensor = &(*sit); // set reference
	}
	// not a sensor, search for construction instance
	else {
		// find construction instance id
		std::vector<NANDRAD::ConstructionInstance>::const_iterator conit =
				std::find(conInstances.begin(), conInstances.end(), m_sensorId);

		if (conit != conInstances.end()) {

			// only constructions with outside boundary are accepted
			if (conit->interfaceAZoneID() != 0 && conit->interfaceBZoneID() != 0) {
				throw IBK::Exception(IBK::FormatString("Construction #%1 is an inside construction "
													   "and may therefore not be referenced.")
									 .arg(m_sensorId), FUNC_ID);
			}

			// set reference to construction instance (contains orientation and inclination)
			m_constructionInstance = &(*conit);
		}
		else {
			// maybe an embedded object is referenced?
			// process all construction instances
			for(const NANDRAD::ConstructionInstance &conInstance : conInstances) {
				// find embedded object id
				std::vector<NANDRAD::EmbeddedObject>::const_iterator embit =
						std::find(conInstance.m_embeddedObjects.begin(),
								  conInstance.m_embeddedObjects.end(),
								  m_sensorId);

				if (embit != conInstance.m_embeddedObjects.end()) {

					// sensor indicates an embedded object, check if this is a construction
					// with outside boundary
					if (conInstance.interfaceAZoneID() != 0 && conInstance.interfaceBZoneID() != 0) {
						throw IBK::Exception(IBK::FormatString("Embedded object with id #%1 is part of an inside construction "
															   "and may therefore not be referenced.")
											 .arg(m_sensorId), FUNC_ID);
					}

					// set reference to embedded object
					m_embeddedObject = &(*embit);
					break; // break loop
				}
			}
		}
	}

	// we must have at least one valid source of radiation intensity referenced!
	if(m_sensor == nullptr && m_constructionInstance == nullptr && m_embeddedObject == nullptr)
		throw IBK::Exception(IBK::FormatString("Neither sensor nor construction instance nor embedded object with id #%1 does exist.")
							 .arg(m_sensorId), FUNC_ID);
}


} // namespace NANDRAD
