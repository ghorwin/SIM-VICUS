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
		// check wall u-value
		m_para[P_UValuePipeWall].checkedValue("UValuePipeWall", "W/mK", "W/mK", 0, false, std::numeric_limits<double>::max(), true,
								"Pipe UValue must be > 0 W/mK.");
	}
}

} // namespace NANDRAD
