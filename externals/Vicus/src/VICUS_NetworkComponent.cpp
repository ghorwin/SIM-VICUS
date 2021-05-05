#include "VICUS_NetworkComponent.h"

#include <NANDRAD_HydraulicNetworkComponent.h>

namespace VICUS {


bool NetworkComponent::isValid() const {

	std::vector<unsigned int> paraVec = NANDRAD::HydraulicNetworkComponent::requiredParameter(
										static_cast<NANDRAD::HydraulicNetworkComponent::ModelType>(m_modelType), 1);
	for (unsigned int i: paraVec){
		try {
			NANDRAD::HydraulicNetworkComponent::checkModelParameter(m_para[i], i);
		} catch (IBK::Exception) {
			return false;
		}
	}
	return true;
}

AbstractDBElement::ComparisonResult NetworkComponent::equal(const AbstractDBElement *other) const {
	const NetworkComponent * otherNetComp = dynamic_cast<const NetworkComponent*>(other);
	if (otherNetComp == nullptr)
		return Different;

	//first check critical data

	//check parameters
	for(unsigned int i=0; i<NUM_P; ++i){
		if(m_para[i] != otherNetComp->m_para[i])
			return Different;
	}
	if(m_modelType != otherNetComp->m_modelType)
		return Different;

	//check meta data

	if(m_displayName != otherNetComp->m_displayName ||
			m_color != otherNetComp->m_color ||
			m_dataSource != otherNetComp->m_dataSource ||
			m_manufacturer != otherNetComp->m_manufacturer ||
			m_notes != otherNetComp->m_notes)
		return OnlyMetaDataDiffers;

	return Equal;
}


} // namespace VICUS
