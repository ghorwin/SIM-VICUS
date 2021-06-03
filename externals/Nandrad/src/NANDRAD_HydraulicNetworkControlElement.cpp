#include "NANDRAD_HydraulicNetworkControlElement.h"

#include "NANDRAD_KeywordList.h"

#include <algorithm>

namespace NANDRAD {

void HydraulicNetworkControlElement::checkParameters(const std::vector<NANDRAD::Zone> &zones) {
	FUNCID(HydraulicNetworkControlElement::checkParameters);

	// NOTE: the check below is unecessary - should be ensured already through the "xml:required" specification!

	if (m_controlledProperty == NUM_CP)
		throw IBK::Exception("Missing attribute 'controlledProperty'.", FUNC_ID);

	try {
		// check individual configuations for different controller properties
		switch (m_controlledProperty) {
			case CP_TemperatureDifference: {
				if (m_controllerType == NUM_CT)
					throw IBK::Exception("Missing attribute 'controllerType'.", FUNC_ID);

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
				m_para[P_MassFluxSetpoint].checkedValue("MassFluxSetpoint", "kg/s", "kg/s",
																	 0, false, std::numeric_limits<double>::max(), false, nullptr);
			} break;

			case NUM_CP: break; // just to make compiler happy
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

			case NUM_CT: break; // just to make compiler happy
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
			return {CP_ThermostatValue};
		case HydraulicNetworkComponent::MT_HeatExchanger:
			return {CP_TemperatureDifference};
		case HydraulicNetworkComponent::MT_ControlledValve:
			return {CP_MassFlux, CP_TemperatureDifferenceOfFollowingElement};
		case HydraulicNetworkComponent::MT_ConstantPressurePump:
			// we could control mass flux here as well ...
		case HydraulicNetworkComponent::MT_DynamicPipe:
		case HydraulicNetworkComponent::MT_HeatPumpIdealCarnot:
		case HydraulicNetworkComponent::NUM_MT:		// just for compiler
			return {};
	}
}


} // namespace NANDRAD
