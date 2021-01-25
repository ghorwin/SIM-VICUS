#include "NANDRAD_HydraulicNetworkComponent.h"

#include "NANDRAD_HydraulicNetwork.h"
#include "NANDRAD_KeywordList.h"


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


void HydraulicNetworkComponent::checkParameters(int networkModelType) const {

	std::string category = "HydraulicNetworkComponent::para_t";
	// check for all necessary parameters of current model type
	std::vector<unsigned int> para = requiredParameter(m_modelType);

	// filter necessary parameters for network model types
	HydraulicNetwork::ModelType modelType = (HydraulicNetwork::ModelType) networkModelType;

	for(unsigned int i = 0; i < para.size(); ++i) {

		para_t paraEnum = (para_t)(para[i]);
		switch(modelType) {
			case HydraulicNetwork::MT_HydraulicNetwork: {
				// skip thermal only parameters
				if(paraEnum == P_PumpEfficiency)
					continue;
				if(paraEnum == P_MotorEfficiency)
					continue;
				if(paraEnum == P_Volume)
					continue;
				if(paraEnum == P_COP)
					continue;
				if(paraEnum == P_UAValue)
					continue;
			} break;
			default: break;
		}
		// check values: at the moment all are > 0
		m_para[paraEnum].checkedValue(KeywordList::Unit(category.c_str(), paraEnum), KeywordList::Unit(category.c_str(), paraEnum),
							   0, false, std::numeric_limits<double>::max(), true,
							   IBK::FormatString("'%1' must be > 0 %2.")
							   .arg(KeywordList::Keyword(category.c_str(), paraEnum))
							   .arg(KeywordList::Unit(category.c_str(), paraEnum)).str().c_str() );
		// TODO: specify conditions
	}

}

} // namespace NANDRAD
