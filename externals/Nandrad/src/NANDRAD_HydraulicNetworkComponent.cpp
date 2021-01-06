#include "NANDRAD_HydraulicNetworkComponent.h"


namespace NANDRAD {

bool HydraulicNetworkComponent::operator!=(const HydraulicNetworkComponent &other) const {

	if (m_id != other.m_id)									return true;
	if (m_displayName != other.m_displayName)				return true;

	if (!sameParametersAs(other))							return true;

	return false;
}


bool HydraulicNetworkComponent::sameParametersAs(const HydraulicNetworkComponent & other) const {
	for (unsigned n=0; n<NUM_P; ++n){
		if (m_para[n] != other.m_para[n])
			return false;
	}
	if (m_modelType != other.m_modelType)					return false;
	if (m_heatExchangeType != other.m_heatExchangeType)		return false;
	return true;
}

} // namespace NANDRAD
