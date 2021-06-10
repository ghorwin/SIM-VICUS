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

		// check array parameters
		if (m_modelType == MT_HeatPumpRealSourceSide){
			if (m_arrayPara[AP_PolyCoefficientsQCond].size() != 6)
				throw IBK::Exception(IBK::FormatString("There must be exactly 6 values in '%1'").arg(
									KeywordList::Keyword("HydraulicNetworkComponent::arrayPara_t", AP_PolyCoefficientsQCond)),
									FUNC_ID);
			if (m_arrayPara[AP_PolyCoefficientsPEl].size() != 6)
				throw IBK::Exception(IBK::FormatString("There must be exactly 6 values in '%1'").arg(
									KeywordList::Keyword("HydraulicNetworkComponent::arrayPara_t", AP_PolyCoefficientsPEl)),
									FUNC_ID);
		}

		// check optional parameters, if given
		if (!m_para[P_FractionOfMotorInefficienciesToFluidStream].name.empty())
			checkModelParameter(m_para[P_FractionOfMotorInefficienciesToFluidStream], P_FractionOfMotorInefficienciesToFluidStream);
		else
			m_para[P_FractionOfMotorInefficienciesToFluidStream].value = 1; // set default value

		// for MT_SupplyTemperatureAdapter, initialize zeta and diameter with defaults
		if (m_modelType == MT_SupplyTemperatureAdapter) {
			m_para[P_HydraulicDiameter].value=1;
			m_para[P_PressureLossCoefficient].value=0;
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
			case MT_HeatPumpIdealCarnotSupplySide:
			case MT_HeatPumpIdealCarnotSourceSide:
			case MT_HeatPumpRealSourceSide:
				return {P_PressureLossCoefficient, P_HydraulicDiameter};
			case MT_HeatExchanger:
				return {P_PressureLossCoefficient, P_HydraulicDiameter};
			case MT_DynamicPipe:
				return {P_PipeMaxDiscretizationWidth};
			case MT_SimplePipe:
				return {};
			case MT_ControlledValve:
				return {P_PressureLossCoefficient, P_HydraulicDiameter};
			case MT_ControlledPump:
			case MT_SupplyTemperatureAdapter: // no parameters needed
			case NUM_MT:
				return {};
		}
	}
	// Thermo-Hydraulic network with heat exchange
	else {
		switch (modelType) {
			case MT_ConstantPressurePump:
				return {P_PressureHead, P_PumpEfficiency, P_Volume}; // Note: P_FractionOfMotorInefficienciesToFluidStream is optional and defaults to 1
			case MT_ConstantMassFluxPump :
				return {P_MassFlux, P_PumpEfficiency, P_Volume};
			case MT_ControlledPump:
				return {P_PumpEfficiency, P_Volume}; // Note: P_FractionOfMotorInefficienciesToFluidStream is optional and defaults to 1
			case MT_HeatPumpIdealCarnotSupplySide:
			case MT_HeatPumpIdealCarnotSourceSide:
				return {P_PressureLossCoefficient, P_HydraulicDiameter, P_Volume, P_CarnotEfficiency, P_MaximumHeatingPower};
			case MT_HeatExchanger:
				return {P_PressureLossCoefficient, P_HydraulicDiameter, P_Volume};
			case MT_DynamicPipe:
				return {P_PipeMaxDiscretizationWidth};
			case MT_SimplePipe:
				return {};
			case MT_ControlledValve:
				return {P_PressureLossCoefficient, P_HydraulicDiameter, P_Volume};
			case MT_HeatPumpRealSourceSide:
				return {P_PressureLossCoefficient, P_HydraulicDiameter, P_Volume};
			case MT_SupplyTemperatureAdapter: // no parameters needed
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
		case P_MaximumHeatingPower:
		case P_PipeMaxDiscretizationWidth:{
			para.checkedValue(name, unit, unit, 0, false, std::numeric_limits<double>::max(), true, nullptr);
			break;
		}
		// value must be >0 and <1
		case P_CarnotEfficiency:
		case P_PumpEfficiency:
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
	}
}



} // namespace NANDRAD
