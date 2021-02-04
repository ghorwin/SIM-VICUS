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

	// get all necessary parameters of current model type
	std::vector<unsigned int> para = requiredParameter(m_modelType, networkModelType);

	// check the parameters
	for (unsigned int i: para){
		checkModelParameter(m_para[i], i);
	}
}


std::vector<unsigned int> HydraulicNetworkComponent::requiredParameter(const HydraulicNetworkComponent::ModelType modelType,
																	   int networkModelType){
	HydraulicNetwork::ModelType netModelType = (HydraulicNetwork::ModelType) networkModelType;
	if (netModelType == HydraulicNetwork::MT_HydraulicNetwork){
		switch (modelType) {
			case MT_ConstantPressurePumpModel:
				return {P_PressureHead};
			case MT_HeatPump:
				return {P_PressureLossCoefficient, P_HydraulicDiameter, P_COP};
			case MT_HeatExchanger:
				return {P_PressureLossCoefficient, P_HydraulicDiameter};
			case MT_DynamicPipe:
			case MT_DynamicAdiabaticPipe:
				return {P_PipeMaxDiscretizationWidth};
			default:
				return {};
		}
	}
	else {
		switch (modelType) {
			case MT_ConstantPressurePumpModel:
				return {P_PressureHead, P_PumpEfficiency, P_MotorEfficiency, P_Volume};
			case MT_HeatPump:
				return {P_PressureLossCoefficient, P_HydraulicDiameter, P_COP, P_Volume};
			case MT_HeatExchanger:
				return {P_PressureLossCoefficient, P_HydraulicDiameter, P_Volume};
			case MT_DynamicPipe:
			case MT_DynamicAdiabaticPipe:
				return {P_PipeMaxDiscretizationWidth};
			default:
				return {};
		}
	}
}


void HydraulicNetworkComponent::checkModelParameter(const IBK::Parameter &para, const unsigned int numPara)
{
	const char * enumName = "HydraulicNetworkComponent::para_t";
	const char * name = KeywordList::Keyword(enumName, (int)numPara);
	const char * unit = KeywordList::Unit(enumName, (int)numPara);

	switch (numPara) {
		// value must be >0
		case P_HydraulicDiameter:
		case P_PressureLossCoefficient:
		case P_ExternalHeatTransferCoefficient:
		case P_Volume:
		case P_UAValue:
		case P_PipeMaxDiscretizationWidth:
		case P_COP:{
			para.checkedValue(name, unit, unit, 0, false, std::numeric_limits<double>::max(), true, nullptr);
			break;
		}
		// value must be >0 and <1
		case P_PumpEfficiency:
		case P_MotorEfficiency: {
			para.checkedValue(name, unit, unit, 0, false, 1.0, true, nullptr);
			break;
		}
		// value can be negative
		case P_PressureHead: {
			para.checkedValue(name, unit, unit, std::numeric_limits<double>::lowest(), true,
							  std::numeric_limits<double>::max(), true, nullptr);
			break;
		}
	}
}


} // namespace NANDRAD
