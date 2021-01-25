#include "NANDRAD_HydraulicNetworkPipeProperties.h"

namespace NANDRAD {

bool HydraulicNetworkPipeProperties::operator!=(const HydraulicNetworkPipeProperties &other) const {
	for (unsigned n=0; n<NUM_P; ++n){
		if (m_para[n] != other.m_para[n])
			return true;
	}
	return false;
}

void HydraulicNetworkPipeProperties::checkParameters() const {
	// check for mandatory and required parameters
	// check for meaningful value ranges
	m_para[P_PipeRoughness].checkedValue("m", "m", 0.0, false, std::numeric_limits<double>::max(), true,
							"Pipe roughness must be > 0 mm.");
	m_para[P_PipeInnerDiameter].checkedValue("m", "m", 0, false, std::numeric_limits<double>::max(), true,
							"Pipe inner diameter must be > 0 mm.");
	m_para[P_PipeOuterDiameter].checkedValue("m", "m", 0, false, std::numeric_limits<double>::max(), true,
							"Pipe outer diameter must be > 0 mm.");

	// TODO : inner < outer

	// lambda
}

} // namespace NANDRAD
