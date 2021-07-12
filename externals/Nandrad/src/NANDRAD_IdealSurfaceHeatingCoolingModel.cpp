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

#include "NANDRAD_IdealSurfaceHeatingCoolingModel.h"

#include <algorithm>

#include "NANDRAD_KeywordList.h"

namespace NANDRAD {

void IdealSurfaceHeatingCoolingModel::checkParameters(const std::vector<NANDRAD::Zone> & zones) {
	FUNCID(IdealSurfaceHeatingCoolingModel::checkParameters);

	// all models require an object list with indication of construction that this model applies to
	if (m_constructionObjectList.empty())
		throw IBK::Exception(IBK::FormatString("Missing 'ConstructionObjectList' parameter."), FUNC_ID);

	// either maximum heating or cooling power must be given
	if (m_para[P_MaxHeatingPowerPerArea].name.empty() && m_para[P_MaxCoolingPowerPerArea].name.empty())
		throw IBK::Exception("Either maximum heating power or maximum cooling power must be defined!", FUNC_ID);

	// if maximum heating power is defined than check value
	if (!m_para[P_MaxHeatingPowerPerArea].name.empty()) {
		m_para[P_MaxHeatingPowerPerArea].checkedValue("MaxHeatingPowerPerArea", "W/m2", "W/m2",
												   0, true,
												   std::numeric_limits<double>::max(), true,
												   "Maximum for heating power must be >= 0 W/m2.");
	}
	// otherwise fill with 0
	else {
		m_para[P_MaxHeatingPowerPerArea].set("MaxHeatingPowerPerArea", 0, IBK::Unit("W/m2"));
	}
	// if maximum cooling power is defined than check value
	if (!m_para[P_MaxCoolingPowerPerArea].name.empty()) {
		m_para[P_MaxCoolingPowerPerArea].checkedValue("MaxCoolingPowerPerArea", "W/m2", "W/m2",
												   0, true,
												   std::numeric_limits<double>::max(), true,
												   "Maximum for cooling power must be >= 0 W/m2.");
	}
	// otherwise fill with 0
	else {
		m_para[P_MaxCoolingPowerPerArea].set("MaxCoolingPowerPerArea", 0, IBK::Unit("W/m2"));
	}

	// resolve thermostat zone
	std::vector<NANDRAD::Zone>::const_iterator zone_it = std::find(zones.begin(), zones.end(), m_thermostatZoneId);

	if (zone_it == zones.end())
		throw IBK::Exception(IBK::FormatString("Invalid/undefined zone with '%1' in ThermostatZoneId.")
							 .arg(m_thermostatZoneId), FUNC_ID);

}

bool IdealSurfaceHeatingCoolingModel::equal(const IdealSurfaceHeatingCoolingModel &other) const {
	//check parameters
	for(unsigned int i=0; i<NUM_P; ++i){
		if(m_para[i] != other.m_para[i])
			return false;
	}

	if(m_thermostatZoneId != other.m_thermostatZoneId)
		return false;

	return true;
}


} // namespace NANDRAD

