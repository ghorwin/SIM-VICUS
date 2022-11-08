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

#include "NANDRAD_HydraulicNetworkHeatExchange.h"

#include "NANDRAD_ConstructionInstance.h"
#include "NANDRAD_KeywordList.h"
#include "NANDRAD_Zone.h"


namespace NANDRAD {


void HydraulicNetworkHeatExchange::checkParameters(const std::map<std::string, IBK::Path> &placeholders,
												   const std::vector<Zone> &zones,
												   const std::vector<ConstructionInstance> &conInstances,
												   bool keepTsvFileValues)
{
	FUNCID(HydraulicNetworkHeatExchange::checkParameters);

	// check parameters required for thermal balances/heat exchange
	// Note: We ONLY check the required PARAMETERS here! We do not check for implemented combinations of model type
	//       and heat exchange type - this is handled when the models are instantiated.

	char * const errmsg = nullptr;
	try {
		// decide which heat exchange is chosen
		switch (m_modelType) {
			case T_TemperatureConstant: {
				// check temperature
				m_para[P_Temperature].checkedValue("Temperature", "C", "C", -200.0, true, std::numeric_limits<double>::max(), true,
												   "Temperature must be >= -200 C.");
				// check for external heat transfer coefficient
				m_para[P_ExternalHeatTransferCoefficient].checkedValue("ExternalHeatTransferCoefficient",
																	   "W/m2K", "W/m2K", 0, true,
																	   std::numeric_limits<double>::max(),
																	   true, errmsg);
			} break;


			case T_TemperatureSpline: {
				// replace place holders
				m_splPara[SPL_Temperature].m_tsvFile = m_splPara[SPL_Temperature].m_tsvFile.withReplacedPlaceholders(placeholders);
				try {
					//  check the spline and convert it to base units automatically
					m_splPara[SPL_Temperature].checkAndInitialize("Temperature", IBK::Unit("s"), IBK::Unit("K"),
																  IBK::Unit("K"), 0, false, std::numeric_limits<double>::max(), false,
																  "Temperature must be > 0 K.");
					if (!keepTsvFileValues) {
						m_splPara[SPL_Temperature].m_name = "Temperature";
						m_splPara[SPL_Temperature].m_values.clear(); // this is necessary to not save both values and tsv-filename to the NANDRAD file
					}
				} catch (IBK::Exception &ex) {
					throw IBK::Exception(ex, IBK::FormatString("Error initializing spline 'Temperature'."), FUNC_ID);
				}

				// check for external heat transfer coefficient
				m_para[P_ExternalHeatTransferCoefficient].checkedValue("ExternalHeatTransferCoefficient",
																	   "W/m2K", "W/m2K", 0, true,
																	   std::numeric_limits<double>::max(),
																	   true, errmsg);
			} break;

			case T_TemperatureSplineEvaporator : {
				// replace place holders
				m_splPara[SPL_Temperature].m_tsvFile = m_splPara[SPL_Temperature].m_tsvFile.withReplacedPlaceholders(placeholders);
				try {
					//  check the spline and convert it to base units automatically
					m_splPara[SPL_Temperature].checkAndInitialize("Temperature", IBK::Unit("s"), IBK::Unit("K"),
																  IBK::Unit("K"), 0, false, std::numeric_limits<double>::max(), false,
																  "Temperature must be > 0 K.");
					if (!keepTsvFileValues) {
						m_splPara[SPL_Temperature].m_name = "Temperature";
						m_splPara[SPL_Temperature].m_values.clear(); // this is necessary to not save both values and tsv-filename to the NANDRAD file
					}
				} catch (IBK::Exception &ex) {
					throw IBK::Exception(ex, IBK::FormatString("Error initializing spline 'Temperature'."), FUNC_ID);
				}
			} break;

			case T_TemperatureZone: {
				// check for zone id
				unsigned int zoneId = m_idReferences[ID_ZoneId];
				if (zoneId == INVALID_ID)
					throw IBK::Exception(IBK::FormatString("Missing ID reference 'ZoneId'."), FUNC_ID);

				// find zone id
				if (std::find(zones.begin(), zones.end(), zoneId) == zones.end())
					throw IBK::Exception(IBK::FormatString("Zone with id %1 does not exist.").arg(zoneId), FUNC_ID);

				// check for external heat transfer coefficient
				m_para[P_ExternalHeatTransferCoefficient].checkedValue("ExternalHeatTransferCoefficient",
																	   "W/m2K", "W/m2K", 0, true,
																	   std::numeric_limits<double>::max(),
																	   true, nullptr);
			} break;

			case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstructionLayer: {
				// check for construction instance id
				unsigned int conInstanceId = m_idReferences[ID_ConstructionInstanceId];
				// check for construction instance id
				if (conInstanceId == INVALID_ID)
					throw IBK::Exception(IBK::FormatString("Missing ID reference 'ConstructionInstanceId'."), FUNC_ID);
				// find zone id
				if (std::find(conInstances.begin(), conInstances.end(), conInstanceId) == conInstances.end())
					throw IBK::Exception(IBK::FormatString("ConstructionInstance with id %1 does not exist.").arg(conInstanceId), FUNC_ID);
			}
			break;

			case T_HeatLossConstant: {
				// check heat flux
				m_para[P_HeatLoss].checkedValue("HeatLoss", "W", "W",
												std::numeric_limits<double>::lowest(), false, std::numeric_limits<double>::max(), false, nullptr);
			} break;

			case T_HeatLossSpline:
			case T_HeatLossSplineCondenser:
			case T_HeatingDemandSpaceHeating: {
				// replace place holders
				m_splPara[SPL_HeatLoss].m_tsvFile = m_splPara[SPL_HeatLoss].m_tsvFile.withReplacedPlaceholders(placeholders);
				try {
					//  check the spline and convert it to base units automatically
					m_splPara[SPL_HeatLoss].checkAndInitialize("HeatLoss", IBK::Unit("s"), IBK::Unit("J/s"),
															   IBK::Unit("J/s"), std::numeric_limits<double>::lowest(), false,
															   std::numeric_limits<double>::max(), false,
															   errmsg);
					if (!keepTsvFileValues) {
						m_splPara[SPL_HeatLoss].m_name = "HeatLoss";
						m_splPara[SPL_HeatLoss].m_values.clear();
					}
				} catch (IBK::Exception &ex) {
					throw IBK::Exception(ex, IBK::FormatString("Error initializing spline 'HeatLoss'."), FUNC_ID);
				}

			} break;

			case NUM_T:
				// No thermal exchange, nothing to initialize
			break;

		} // switch

	} catch (IBK::Exception & ex) {
		std::string errMsgStr;
		if (errmsg!=nullptr) {
			errMsgStr = *errmsg;
			throw IBK::Exception(ex, IBK::FormatString("Missing/invalid parameters for heat exchange model '%1':\n%2.")
				 .arg(KeywordList::Keyword("HydraulicNetworkHeatExchange::ModelType", m_modelType)).arg(errMsgStr),
				 FUNC_ID);
		}
		else
			throw IBK::Exception(ex, IBK::FormatString("Missing/invalid parameters for heat exchange model '%1'.")
				 .arg(KeywordList::Keyword("HydraulicNetworkHeatExchange::ModelType", m_modelType)),
				 FUNC_ID);
	}
}



bool HydraulicNetworkHeatExchange::operator!=(const HydraulicNetworkHeatExchange &other) const {
	if (m_modelType != other.m_modelType)
		return true;

	for (unsigned int i=0; i<NUM_P; ++i){
		if (m_para[i] != other.m_para[i])
			return true;
	}

	for (unsigned int i=0; i<NUM_SPL; ++i){
		if (m_splPara[i] != other.m_splPara[i])
			return true;
	}

	for (unsigned int i=0; i<NUM_ID; ++i){
		if (m_idReferences[i] != other.m_idReferences[i])
			return true;
	}

	return false;
}



std::vector<HydraulicNetworkHeatExchange::ModelType> NANDRAD::HydraulicNetworkHeatExchange::availableHeatExchangeTypes(
		const NANDRAD::HydraulicNetworkComponent::ModelType modelType)
{
	// some models may be adiabatic, hence we also return NUM_T as available heat exchange type
	switch (modelType) {
		case HydraulicNetworkComponent::MT_SimplePipe:
			return {NUM_T, T_TemperatureConstant, T_TemperatureSpline, T_HeatLossConstant, T_HeatLossSpline, T_TemperatureZone, T_TemperatureConstructionLayer};
		case HydraulicNetworkComponent::MT_DynamicPipe:
			return {NUM_T, T_TemperatureConstant, T_TemperatureSpline, T_TemperatureZone, T_TemperatureConstructionLayer};
		case HydraulicNetworkComponent::MT_HeatPumpVariableIdealCarnotSourceSide:
		case HydraulicNetworkComponent::MT_HeatPumpVariableSourceSide:
			return {T_HeatLossSplineCondenser};  // must not be adiabatic
//		case HydraulicNetworkComponent::MT_HeatPumpOnOffSourceSideWithBuffer:
//			return {T_HeatingDemandSpaceHeating};  // must not be adiabatic
		case HydraulicNetworkComponent::MT_HeatExchanger:
			return {T_HeatLossConstant, T_HeatLossSpline}; // must not be adiabatic
		case HydraulicNetworkComponent::MT_HeatPumpVariableIdealCarnotSupplySide:
		case HydraulicNetworkComponent::MT_ConstantPressurePump:
		case HydraulicNetworkComponent::MT_ConstantMassFluxPump:
		case HydraulicNetworkComponent::MT_VariablePressurePump:
		case HydraulicNetworkComponent::MT_ControlledPump:
		case HydraulicNetworkComponent::MT_ControlledValve:
		case HydraulicNetworkComponent::MT_HeatPumpOnOffSourceSide:
		case HydraulicNetworkComponent::MT_IdealHeaterCooler:
		case HydraulicNetworkComponent::MT_ConstantPressureLossValve:
		case HydraulicNetworkComponent::MT_PressureLossElement:
			return {NUM_T};
		case HydraulicNetworkComponent::NUM_MT: ; // just to make compiler happy
	}
	return {};
}



} // namespace NANDRAD
