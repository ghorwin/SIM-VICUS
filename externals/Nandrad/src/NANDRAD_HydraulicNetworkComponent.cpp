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

		// get all necessary spline parameters of current model type
		std::vector<unsigned int> paraSpl = requiredSplineParameter(m_modelType, m_heatPumpIntegration);
		// check the parameters
		for (unsigned int i: paraSpl){
			checkModelSplineParameter(m_splPara[i], i);
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
						P_HeatPumpNominalTemperatureDifference, P_MaximumHeatHeatingPower};
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



std::vector<unsigned int> HydraulicNetworkComponent::requiredSplineParameter(const HydraulicNetworkComponent::ModelType modelType,
																			 const HydraulicNetworkComponent::HeatPumpIntegration heatPumpIntegration){

	switch (modelType) {

		case MT_HeatPumpIdealCarnot:
			switch (heatPumpIntegration) {
				case HydraulicNetworkComponent::HP_SourceSide :
					return {SPL_CondenserMeanTemperature};
				case HydraulicNetworkComponent::HP_SupplySide :
				case HydraulicNetworkComponent::HP_SupplyAndSourceSide :
				case HydraulicNetworkComponent::NUM_HP:
					;
			}
		case MT_HeatPumpReal:
		case MT_ConstantPressurePump:
		case MT_HeatExchanger:
		case MT_DynamicPipe:
		case MT_SimplePipe:
		case NUM_MT: ;
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
		case P_MaximumHeatHeatingPower:
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

void HydraulicNetworkComponent::checkModelSplineParameter(LinearSplineParameter &paraSpl, const unsigned int numPara)
{
	FUNCID("HydraulicNetworkComponent::checkModelSplineParameter");

	const char * enumName = "HydraulicNetworkComponent::splinePara_t";
	const char * name = KeywordList::Keyword(enumName, (int)numPara);
	const char * unit = KeywordList::Unit(enumName, (int)numPara);

	try {

		switch (numPara) {

			// can be any value
			case SPL_CondenserMeanTemperature:
			case SPL_EvaporatorMeanTemperature:
			case SPL_CondenserOutletSetPointTemperature: {
				paraSpl.checkAndInitialize(name, IBK::Unit("s"), IBK::Unit("K"),
											IBK::Unit(unit), std::numeric_limits<double>::min(), false,
											std::numeric_limits<double>::max(), false,
											nullptr);
			} break;

			// must be between 0 and 1
			case SPL_HeatPumpControlSignal:{
				paraSpl.checkAndInitialize(name, IBK::Unit("s"), IBK::Unit(unit),
											IBK::Unit(unit), 0, true, 1, true, nullptr);
			}
		}

	} catch (IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error initializing spline '%1'.").arg(name), FUNC_ID);
	}

}


} // namespace NANDRAD
