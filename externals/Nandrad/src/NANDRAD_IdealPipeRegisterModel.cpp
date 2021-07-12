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

#include "NANDRAD_IdealPipeRegisterModel.h"

#include "NANDRAD_KeywordList.h"
#include "NANDRAD_Zone.h"
#include "NANDRAD_HydraulicNetwork.h"

#include <algorithm>

namespace NANDRAD {

void IdealPipeRegisterModel::checkParameters(const std::vector<NANDRAD::Zone> &zones) {
	FUNCID(IdealPipeRegisterModel::checkParameters);

	// all models require an object list with indication of construction instances that this model applies to
	if (m_constructionObjectList.empty())
		throw IBK::Exception(IBK::FormatString("Missing 'ConstructionObjectList' parameter."), FUNC_ID);

	m_para[P_PipeLength].checkedValue("PipeLength", "m", "m",
											   0, false,
											   std::numeric_limits<double>::max(), true,
											   "Pipe lenght must be > 0 m.");
	m_para[P_PipeInnerDiameter].checkedValue("PipeInnerDiameter", "mm", "mm",
											   0, false,
											   std::numeric_limits<double>::max(), true,
											   "Pipe inner diameter flow must be > 0 mm.");
	m_para[P_UValuePipeWall].checkedValue("UValuePipeWall", "W/mK", "W/mK",
											   0, false,
											   std::numeric_limits<double>::max(), true,
											   "Pipe wall U-value must be > 0 W/mK.");

	// decide how to recieve supply temperature
	if (m_modelType == MT_Constant) {
		m_para[P_SupplyTemperature].checkedValue("SupplyTemperature", "C", "K",
												   0.0, true,
												   std::numeric_limits<double>::max(), true,
												   "Supply temperature must be >= 0 K.");
		// check parameters
		m_para[P_MaxMassFlux].checkedValue("MaxMassFlux", "kg/s", "kg/s",
												   0, false,
												   std::numeric_limits<double>::max(), true,
												   "Maximum mass flow must be > 0 kg/s.");
	}

	// set default value for int parameters
	if (m_intPara[IP_NumberParallelPipes].name.empty()) {
		// one pipe is default
		m_intPara[IP_NumberParallelPipes].set("NumberParallelPipes", 1);
	}
	else {
		// check that given number is >= 1
		m_intPara[IP_NumberParallelPipes].checkIfValueIsAboveLimit(IBK::IntPara("NumberParallelPipes", 1), true);
	}

	// check validity of thermostat zone
	std::vector<NANDRAD::Zone>::const_iterator zone_it = std::find(zones.begin(), zones.end(), m_thermostatZoneId);

	if (zone_it == zones.end())
		throw IBK::Exception(IBK::FormatString("Invalid/undefined zone with '%1' in ThermostatZoneId.")
							 .arg(m_thermostatZoneId), FUNC_ID);

	m_fluid.checkParameters(HydraulicNetwork::MT_ThermalHydraulicNetwork);
}

bool IdealPipeRegisterModel::equal(const IdealPipeRegisterModel &other) const {

	//check parameters
	for(unsigned int i=0; i<NUM_P; ++i){
		if(m_para[i] != other.m_para[i])
			return false;
	}

	//check parameters
	for(unsigned int i=0; i<NUM_IP; ++i){
		if(m_intPara[i] != other.m_intPara[i])
			return false;
	}

	if(m_thermostatZoneId != other.m_thermostatZoneId ||
			m_modelType != other.m_modelType ||
			!m_fluid.equal(other.m_fluid))
		return false;

	return true;
}


} // namespace NANDRAD

