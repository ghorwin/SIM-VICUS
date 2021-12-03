#include "NANDRAD_HydraulicNetworkControlElement.h"

#include "NANDRAD_KeywordList.h"

#include <algorithm>

namespace NANDRAD {

void HydraulicNetworkControlElement::checkParameters(const std::vector<Zone> & zones) const {
	FUNCID(HydraulicNetworkControlElement::checkParameters);

	// NOTE: the check below is unecessary - should be ensured already through the "xml:required" specification!

	if (m_controlledProperty == NUM_CP)
		throw IBK::Exception("Missing attribute 'controlledProperty'.", FUNC_ID);

	if (m_modelType == NUM_MT)
		throw IBK::Exception("Missing attribute 'modelType'.", FUNC_ID);

	if (m_controlledProperty == CP_PumpOperation){
		if (m_controllerType != CT_OnOffController)
			throw IBK::Exception("Controlled property 'PumpOperation' can only be used with 'OnOffController'.", FUNC_ID);
	}
	else {
		if (!(m_controllerType == CT_PController || m_controllerType == CT_PIController || m_controllerType == CT_PIDController))
			throw IBK::Exception(IBK::FormatString("Controlled property '%1' can only be used with 'PController', 'PIController' or 'PIDController'.")
								 .arg(KeywordList::Keyword("HydraulicNetworkControlElement::ControlledProperty", m_controlledProperty)),
								 FUNC_ID);
	}

	try {
		// check individual configuations for different controller properties
		switch (m_controlledProperty) {
			case CP_TemperatureDifference:
			case CP_TemperatureDifferenceOfFollowingElement: {
				if (m_controllerType == NUM_CT)
					throw IBK::Exception("Missing attribute 'controllerType'.", FUNC_ID);

				if (m_modelType == MT_Constant)
					m_para[P_TemperatureDifferenceSetpoint].checkedValue("TemperatureDifferenceSetpoint", "K", "K",
						 0, false, std::numeric_limits<double>::max(), false, nullptr);
			} break;

			case CP_ThermostatValue: {
				if (m_idReferences[ID_ThermostatZoneId] == NANDRAD::INVALID_ID)
					throw IBK::Exception("Missing 'ThermostatZoneId' for controlled property 'ThermostatValue'!", FUNC_ID);

				// check validity of thermostat zone
				std::vector<NANDRAD::Zone>::const_iterator zone_it = std::find(zones.begin(), zones.end(), m_idReferences[ID_ThermostatZoneId]);

				if (zone_it == zones.end())
					throw IBK::Exception(IBK::FormatString("Invalid/undefined zone with '%1' in ThermostatZoneId.")
										 .arg(m_idReferences[ID_ThermostatZoneId]), FUNC_ID);
			} break;

			case CP_MassFlux : {
				// we need mass flux, but > 0 (cannot set mass flux to zero)
				if (m_modelType == MT_Constant)
					m_para[P_MassFluxSetpoint].checkedValue("MassFluxSetpoint", "kg/s", "kg/s",
						 0, false, std::numeric_limits<double>::max(), false, nullptr);
			} break;

			case CP_PumpOperation : {
					m_para[P_HeatLossOfFollowingElementThreshold].checkedValue("HeatLossOfFollowingElementThreshold", "W", "W",
						 0, false, std::numeric_limits<double>::max(), false, nullptr);
			} break;

			case NUM_CP:
				throw IBK::Exception("Missing or invalid attribute 'controlledProperty'.", FUNC_ID);
		}
	}
	catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, "Missing/invalid parameters.", FUNC_ID);
	}

	try {
		// decide which parameters are needed
		switch (m_controllerType) {

			case CT_PController: {
				m_para[P_Kp].checkedValue("Kp", "---", "---", 0, false, std::numeric_limits<double>::max(), true, nullptr);
			} break;

			case CT_PIController: {
				m_para[P_Kp].checkedValue("Kp", "---", "---", 0, false, std::numeric_limits<double>::max(), true, nullptr);
				m_para[P_Ki].checkedValue("Ki", "---", "---", 0, false, std::numeric_limits<double>::max(), true, nullptr);
			} break;

			case CT_PIDController: {
				m_para[P_Kp].checkedValue("Kp", "---", "---", 0, false, std::numeric_limits<double>::max(), true, nullptr);
				m_para[P_Ki].checkedValue("Ki", "---", "---", 0, true, std::numeric_limits<double>::max(), true, nullptr);
				m_para[P_Kd].checkedValue("Kd", "---", "---", 0, true, std::numeric_limits<double>::max(), true, nullptr);
			} break;

			case CT_OnOffController: {
				m_para[P_HeatLossOfFollowingElementThreshold].checkedValue("HeatLossOfFollowingElementThreshold",
																"W", "W", 0, false, std::numeric_limits<double>::max(), true, nullptr);
			} break;

			case NUM_CT:
				throw IBK::Exception("Missing or invalid attribute 'controllerType'.", FUNC_ID);
		}
	}
	catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, "Missing/invalid parameters.", FUNC_ID);
	}
}



std::vector<HydraulicNetworkControlElement::ControlledProperty> HydraulicNetworkControlElement::availableControlledProperties(
																	const HydraulicNetworkComponent::ModelType modelType)
{
	switch (modelType) {
		case HydraulicNetworkComponent::MT_SimplePipe:
		case HydraulicNetworkComponent::MT_DynamicPipe:
			return {CP_ThermostatValue};
		case HydraulicNetworkComponent::MT_HeatExchanger:
			return {CP_TemperatureDifference};
		case HydraulicNetworkComponent::MT_ControlledValve:
			return {CP_MassFlux, CP_TemperatureDifferenceOfFollowingElement};
		case HydraulicNetworkComponent::MT_ControlledPump:
			return {CP_MassFlux, CP_TemperatureDifferenceOfFollowingElement};
		case HydraulicNetworkComponent::MT_ConstantPressurePump:
			return {CP_PumpOperation};
		case HydraulicNetworkComponent::MT_ConstantMassFluxPump :
		case HydraulicNetworkComponent::MT_VariablePressurePump:
		case HydraulicNetworkComponent::MT_HeatPumpIdealCarnotSourceSide:
		case HydraulicNetworkComponent::MT_HeatPumpIdealCarnotSupplySide:
		case HydraulicNetworkComponent::MT_HeatPumpRealSourceSide:
		case HydraulicNetworkComponent::MT_IdealHeaterCooler:
		case HydraulicNetworkComponent::MT_ConstantPressureLossValve:
		case HydraulicNetworkComponent::MT_PressureLossElement:
		case HydraulicNetworkComponent::NUM_MT: ;		// just for compiler
	}
	return {};
}


} // namespace NANDRAD
