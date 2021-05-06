#include "NANDRAD_HydraulicNetworkComponent.h"

#include <algorithm>

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
	return true;
}


void HydraulicNetworkComponent::checkParameters(int networkModelType) {
	FUNCID(HydraulicNetworkComponent::checkParameters);

	try {

		// get all necessary parameters of current model type
		std::vector<unsigned int> para = requiredParameter(m_modelType, networkModelType);
		// check the parameters
		for (unsigned int i: para){
			checkModelParameter(m_para[i], i);
		}

	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Missing/invalid parameters for component '%1' (#%2) of type %3.")
			.arg(m_displayName).arg(m_id)
			.arg(KeywordList::Keyword("HydraulicNetworkComponent::ModelType", m_modelType)), FUNC_ID);
	}
}


std::vector<unsigned int> HydraulicNetworkComponent::requiredParameter(const HydraulicNetworkComponent::ModelType modelType,
																	   int networkModelType) {
	HydraulicNetwork::ModelType netModelType = (HydraulicNetwork::ModelType) networkModelType;

	// Hydraulic network with constant temperature
	if (netModelType == HydraulicNetwork::MT_HydraulicNetwork){
		switch (modelType) {
			case MT_ConstantPressurePump:
				return {P_PressureHead};
			case MT_HeatPumpIdealCarnot:
				return {P_PressureLossCoefficient, P_HydraulicDiameter};
			case MT_HeatExchanger:
				return {P_PressureLossCoefficient, P_HydraulicDiameter};
			case MT_DynamicPipe:
				return {P_PipeMaxDiscretizationWidth};
			case MT_SimplePipe:
				return {};
			case NUM_MT:
				return {};
		}
	}
	// Thermo-Hydraulic network with heat exchange
	else {
		switch (modelType) {
			case MT_ConstantPressurePump:
				return {P_PressureHead, P_PumpEfficiency, P_Volume};
			case MT_HeatPumpIdealCarnot:
				return {P_PressureLossCoefficient, P_HydraulicDiameter, P_Volume, P_CarnotEfficiency,
						P_HeatPumpNominalTemperatureDifference, P_MaximumHeatingPower};
			case MT_HeatExchanger:
				return {P_PressureLossCoefficient, P_HydraulicDiameter, P_Volume};
			case MT_DynamicPipe:
				return {P_PipeMaxDiscretizationWidth};
			case MT_SimplePipe:
				return {};
			case NUM_MT: ;
		}
	}
	return {};
}


void HydraulicNetworkComponent::checkModelParameter(const IBK::Parameter &para, const unsigned int numPara) {
	const char * enumName = "HydraulicNetworkComponent::para_t";
	const char * name = KeywordList::Keyword(enumName, (int)numPara);
	const char * unit = KeywordList::Unit(enumName, (int)numPara);

	switch (numPara) {
		// value must be >0
		case P_HydraulicDiameter:
		case P_PressureLossCoefficient:
		case P_Volume:
		case P_HeatPumpNominalTemperatureDifference:
		case P_MaximumHeatingPower:
		case P_PipeMaxDiscretizationWidth:{
			para.checkedValue(name, unit, unit, 0, false, std::numeric_limits<double>::max(), true, nullptr);
			break;
		}
		// value must be >0 and <1
		case P_CarnotEfficiency:
		case P_PumpEfficiency: {
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
