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
	if (m_heatExchangeType != other.m_heatExchangeType)		return false;
	return true;
}


void HydraulicNetworkComponent::checkParameters(int networkModelType) const {
	FUNCID(HydraulicNetworkComponent::checkParameters);

	try {

		// get all necessary parameters of current model type
		std::vector<unsigned int> para = requiredParameter(m_modelType, m_heatExchangeType, networkModelType);

		// check the parameters
		for (unsigned int i: para){
			checkModelParameter(m_para[i], i);
		}

		if (networkModelType == HydraulicNetwork::MT_ThermalHydraulicNetwork) {
			if (m_heatExchangeType != NUM_HT) {
				// check if heat exchange type is supported
				std::vector<unsigned int> availableHXTypes = availableHeatExchangeTypes(m_modelType);
				if (std::find(availableHXTypes.begin(), availableHXTypes.end(), m_heatExchangeType) == availableHXTypes.end())
					throw IBK::Exception(IBK::FormatString("Heat exchange type %1 is not allowed for this component.")
									 .arg(KeywordList::Keyword("HydraulicNetworkComponent::HeatExchangeType", m_heatExchangeType))
									 , FUNC_ID);
			}
		}


	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Missing/invalid parameters for component '%1' (#%2) of type %3.")
			.arg(m_displayName).arg(m_id)
			.arg(KeywordList::Keyword("HydraulicNetworkComponent::ModelType", m_modelType)), FUNC_ID);
	}
}


std::vector<unsigned int> HydraulicNetworkComponent::requiredParameter(const HydraulicNetworkComponent::ModelType modelType,
																	   int heatExchangeType,
																	   int networkModelType){
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
				return {P_PressureLossCoefficient, P_HydraulicDiameter, P_Volume, P_CarnotEfficiency};
			case MT_HeatExchanger:
				return {P_PressureLossCoefficient, P_HydraulicDiameter, P_Volume};
			case MT_DynamicPipe: {
				switch(heatExchangeType) {
					case HT_HeatFluxConstant:
					case HT_HeatFluxDataFile:
						return {P_PipeMaxDiscretizationWidth};
					case HT_TemperatureConstant:
					case HT_TemperatureDataFile:
						return {P_PipeMaxDiscretizationWidth, P_ExternalHeatTransferCoefficient};
				}
			}
			case MT_SimplePipe: {
				switch(heatExchangeType) {
					case HT_HeatFluxConstant:
					case HT_HeatFluxDataFile:
						return {};
					case HT_TemperatureConstant:
					case HT_TemperatureDataFile:
						return {P_ExternalHeatTransferCoefficient};
				}
			}
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
		case P_ExternalHeatTransferCoefficient:
		case P_Volume:
		case P_UAValue:
		case P_PipeMaxDiscretizationWidth:{
			para.checkedValue(name, unit, unit, 0, false, std::numeric_limits<double>::max(), true, nullptr);
			break;
		}
		// value must be >0 and <1
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
