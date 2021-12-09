/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "NANDRAD_HydraulicNetworkComponent.h"

#include <algorithm>

#include "NANDRAD_HydraulicNetwork.h"
#include "NANDRAD_KeywordList.h"


namespace NANDRAD {

bool HydraulicNetworkComponent::operator!=(const HydraulicNetworkComponent &other) const {

	if (m_id != other.m_id)									return true;
	if (m_displayName != other.m_displayName)				return true;
	if (m_modelType != other.m_modelType)					return true;

	if (!sameParametersAs(other))							return true;

	return false;
}


bool HydraulicNetworkComponent::sameParametersAs(const HydraulicNetworkComponent & other) const {
	for (unsigned n=0; n<NUM_P; ++n){
		if (m_para[n] != other.m_para[n])
			return false;
	}
	if (m_modelType != other.m_modelType)					return false;
	if (m_polynomCoefficients != other.m_polynomCoefficients) return false;
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

		// check data table
		if (m_modelType == MT_HeatPumpOnOffSourceSide) {
			if (m_polynomCoefficients.m_values["QdotCondensator"].size() != 6)
				throw IBK::Exception("'HeatPumpRealSourceSide' requires polynom coefficient parameter 'QdotCondensator' with exactly 6 values.", FUNC_ID);
			if (m_polynomCoefficients.m_values["Pel"].size() != 6)
				throw IBK::Exception("'HeatPumpRealSourceSide' requires polynom coefficient parameter 'Pel' with exactly 6 values.", FUNC_ID);
		}
		if (m_modelType == MT_HeatPumpVariableSourceSide) {
			if (m_polynomCoefficients.m_values["COP"].size() != 6)
				throw IBK::Exception("'HeatPumpRealSourceSide' requires polynom coefficient parameter 'QdotCondensator' with exactly 6 values.", FUNC_ID);
		}

		// check optional parameters, if given
		if (!m_para[P_FractionOfMotorInefficienciesToFluidStream].name.empty())
			checkModelParameter(m_para[P_FractionOfMotorInefficienciesToFluidStream], P_FractionOfMotorInefficienciesToFluidStream);
		else
			m_para[P_FractionOfMotorInefficienciesToFluidStream].value = 1; // set default value

		// for MT_IdealHeaterCooler, initialize zeta and diameter with defaults
		if (m_modelType == MT_IdealHeaterCooler) {
			m_para[P_HydraulicDiameter].value=1;
			m_para[P_PressureLossCoefficient].value=0;
		}
		// for MT_ConstantPressurePump and MT_VariablePressurePump, we enforce existance of complete parameter set (pump efficiency,
		// maximum pressure head and maximum electric power) once one of the parameters or maximum pressure head and maximum electric power
		// is given
		else if (m_modelType == MT_ConstantPressurePump || m_modelType == MT_VariablePressurePump) {
			// general case
			if(!m_para[NANDRAD::HydraulicNetworkComponent::P_PumpMaximumElectricalPower].name.empty() &&
					m_para[NANDRAD::HydraulicNetworkComponent::P_MaximumPressureHead].empty())
				throw IBK::Exception("Missing paramneter 'MaximumPressureHead'!", FUNC_ID);
			if(!m_para[NANDRAD::HydraulicNetworkComponent::P_MaximumPressureHead].name.empty() &&
					m_para[NANDRAD::HydraulicNetworkComponent::P_PumpMaximumElectricalPower].empty())
				throw IBK::Exception("Missing paramneter 'PumpMaximumElectricalPower'!", FUNC_ID);

			if(networkModelType == HydraulicNetwork::MT_HydraulicNetwork) {
				// special case hydraulic network: pumpm efficiency is not requested per default
				if(!m_para[NANDRAD::HydraulicNetworkComponent::P_PumpMaximumElectricalPower].name.empty() &&
						m_para[NANDRAD::HydraulicNetworkComponent::P_PumpEfficiency].empty())
					throw IBK::Exception("Missing paramneter 'PumpEfficiency'!", FUNC_ID);
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
																	   int networkModelType)
{
	HydraulicNetwork::ModelType netModelType = (HydraulicNetwork::ModelType) networkModelType;

	// Hydraulic network with constant temperature
	if (netModelType == HydraulicNetwork::MT_HydraulicNetwork){
		switch (modelType) {
			case MT_ConstantPressurePump:
				return {P_PressureHead};
			case MT_ConstantMassFluxPump :
				return {P_MassFlux};
			case MT_ControlledPump:
			case MT_VariablePressurePump:
				return {};
			case HydraulicNetworkComponent::MT_HeatPumpVariableIdealCarnotSourceSide:
			case HydraulicNetworkComponent::MT_HeatPumpVariableIdealCarnotSupplySide:
			case HydraulicNetworkComponent::MT_HeatPumpVariableSourceSide:
			case HydraulicNetworkComponent::MT_HeatPumpOnOffSourceSide:
			case MT_HeatExchanger:
			case MT_PressureLossElement:
			case MT_ControlledValve:
				return {P_PressureLossCoefficient, P_HydraulicDiameter};
			case MT_DynamicPipe:
				return {P_PipeMaxDiscretizationWidth};
			case MT_SimplePipe:
				return {};
			case MT_ConstantPressureLossValve:
				return {P_PressureLoss};
			case MT_IdealHeaterCooler: // no parameters needed
			case NUM_MT:
				return {};
		}
	}
	// Thermo-Hydraulic network with heat exchange
	else {
		switch (modelType) {
			case MT_ConstantPressurePump:
				return {P_PressureHead, P_PumpEfficiency, P_Volume}; // Note: P_FractionOfMotorInefficienciesToFluidStream is optional and defaults to 1
			case MT_VariablePressurePump:
				// Note: P_FractionOfMotorInefficienciesToFluidStream is optional and defaults to 1
				return {P_PumpEfficiency, P_Volume, P_DesignPressureHead, P_DesignMassFlux, P_PressureHeadReductionFactor};
			case MT_ConstantMassFluxPump :
				return {P_MassFlux, P_PumpEfficiency, P_Volume};
			case MT_ControlledPump:
				// Note: P_FractionOfMotorInefficienciesToFluidStream is optional and defaults to 1
				return {P_PumpEfficiency, P_Volume, P_PumpMaximumElectricalPower, P_MaximumPressureHead};
			case MT_HeatPumpVariableIdealCarnotSupplySide:
			case MT_HeatPumpVariableIdealCarnotSourceSide:
				return {P_PressureLossCoefficient, P_HydraulicDiameter, P_Volume, P_CarnotEfficiency, P_MaximumHeatingPower};
			case MT_HeatExchanger:
			case MT_PressureLossElement:
			case MT_ControlledValve:
			case MT_HeatPumpVariableSourceSide:
			case MT_HeatPumpOnOffSourceSide:
				return {P_PressureLossCoefficient, P_HydraulicDiameter, P_Volume};
			case MT_DynamicPipe:
				return {P_PipeMaxDiscretizationWidth};
			case MT_SimplePipe:
				return {};
			case MT_ConstantPressureLossValve:
				return {P_PressureLoss, P_Volume};
			case MT_IdealHeaterCooler: // no parameters needed
			case NUM_MT: ;
		}
	}
	return {};
}


std::vector<std::string> HydraulicNetworkComponent::requiredScheduleNames(const HydraulicNetworkComponent::ModelType modelType) {
	switch (modelType)	{
		case MT_HeatPumpVariableIdealCarnotSourceSide:
		case MT_HeatPumpVariableSourceSide:
			return {"CondenserMeanTemperatureSchedule [C]"};
		case MT_HeatPumpVariableIdealCarnotSupplySide:
			return {"CondenserOutletSetpointSchedule [C]"};
		case MT_HeatPumpOnOffSourceSide:
			return {"CondenserOutletSetpointSchedule [C]", "HeatPumpOnOffSignalSchedule [---]"};
		case MT_IdealHeaterCooler:
			return {"SupplyTemperatureSchedule [C]"};
		case MT_ConstantPressurePump:
		case MT_ConstantMassFluxPump :
		case MT_ControlledPump:
		case MT_VariablePressurePump:
		case MT_HeatExchanger:
		case MT_DynamicPipe:
		case MT_SimplePipe:
		case MT_ControlledValve:
		case MT_ConstantPressureLossValve:
		case MT_PressureLossElement:
		case NUM_MT: ;
	}
	return {};
}


void HydraulicNetworkComponent::checkModelParameter(const IBK::Parameter &para, const unsigned int numPara) {
	const char * enumName = "HydraulicNetworkComponent::para_t";
	const char * name = KeywordList::Keyword(enumName, (int)numPara);
	const char * unit = KeywordList::Unit(enumName, (int)numPara);

	switch ((para_t)numPara) {
		// value must be >0
		case P_HydraulicDiameter:
		case P_Volume:
		case P_MaximumHeatingPower:
		case P_PipeMaxDiscretizationWidth:
		case P_DesignMassFlux:
		case P_DesignPressureHead:
		{
			para.checkedValue(name, unit, unit, 0, false, std::numeric_limits<double>::max(), true, nullptr);
			break;
		}
		// value must be >= 0
		case P_MaximumPressureHead:
		case P_PumpMaximumElectricalPower:
		case P_MassFlux:
		case P_PressureLoss:
		case P_PressureLossCoefficient:{
			para.checkedValue(name, unit, unit, 0, true, std::numeric_limits<double>::max(), true, nullptr);
			break;
		}
		// value must be >0 and <=1
		case P_CarnotEfficiency:
		case P_PumpEfficiency:
		case P_PressureHeadReductionFactor:
		case P_FractionOfMotorInefficienciesToFluidStream: {
			para.checkedValue(name, unit, unit, 0, false, 1.0, true, nullptr);
			break;
		}
			// value can be negative
		case P_PressureHead: {
			para.checkedValue(name, unit, unit, std::numeric_limits<double>::lowest(), true,
							  std::numeric_limits<double>::max(), true, nullptr);
			break;
		}

		case NUM_P: break;
	}
}



} // namespace NANDRAD
