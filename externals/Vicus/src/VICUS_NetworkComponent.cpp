#include "VICUS_NetworkComponent.h"

#include <NANDRAD_HydraulicNetworkComponent.h>

namespace VICUS {


bool NetworkComponent::isValid() const {

	std::vector<unsigned int> paraVec = NANDRAD::HydraulicNetworkComponent::requiredParameter(
										static_cast<NANDRAD::HydraulicNetworkComponent::ModelType>(m_modelType),
										static_cast<NANDRAD::HydraulicNetworkComponent::HeatExchangeType>(m_heatExchangeType), 1);
	for (unsigned int i: paraVec){
		try {
			NANDRAD::HydraulicNetworkComponent::checkModelParameter(m_para[i], i);
		} catch (IBK::Exception) {
			return false;
		}
	}
	return true;
}


} // namespace VICUS
