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

#include "NANDRAD_HydraulicNetworkPipeProperties.h"

#include "NANDRAD_HydraulicNetwork.h"

namespace NANDRAD {

bool HydraulicNetworkPipeProperties::operator!=(const HydraulicNetworkPipeProperties &other) const {
	for (unsigned n=0; n<NUM_P; ++n){
		if (m_para[n] != other.m_para[n])
			return true;
	}
	return false;
}


void HydraulicNetworkPipeProperties::checkParameters(int networkModelType) const {
	FUNCID(HydraulicNetworkPipeProperties::checkParameters);

	// check for mandatory and required parameters, check value ranges
	m_para[P_PipeRoughness].checkedValue("PipeRoughness", "m", "m", 0.0, false, std::numeric_limits<double>::max(), true,
							"Pipe roughness must be > 0 mm.");
	m_para[P_PipeInnerDiameter].checkedValue("PipeInnerDiameter", "m", "m", 0, false, std::numeric_limits<double>::max(), true,
							"Pipe inner diameter must be > 0 mm.");
	m_para[P_PipeOuterDiameter].checkedValue("PipeOuterDiameter", "m", "m", 0, false, std::numeric_limits<double>::max(), true,
							"Pipe outer diameter must be > 0 mm.");
	// check if outer parameter is larger than inner parameter
	if (m_para[P_PipeInnerDiameter].value >= m_para[P_PipeOuterDiameter].value)
		throw IBK::Exception("Pipe outer diameter must be larger than pipe inner parameter.", FUNC_ID);

	// check thermal properties
	HydraulicNetwork::ModelType modelType = (HydraulicNetwork::ModelType) networkModelType;

	if (modelType == HydraulicNetwork::MT_ThermalHydraulicNetwork) {
		m_para[P_UValueWall].checkedValue("UValueWall", "W/mK", "W/mK", 0, false, std::numeric_limits<double>::max(), true,
								"Pipe UValue must be > 0 W/mK.");
		m_para[P_DensityWall].checkedValue("DensityWall", "kg/m3", "kg/m3", 0, false, std::numeric_limits<double>::max(), true,
								"Pipe wall density must be > 0 kg/m3.");
		m_para[P_HeatCapacityWall].checkedValue("HeatCapacityWall", "J/kgK", "J/kgK", 0, false, std::numeric_limits<double>::max(), true,
								"Pipe wall heat capcity must be > 0 J/kgK.");
	}
}

} // namespace NANDRAD
