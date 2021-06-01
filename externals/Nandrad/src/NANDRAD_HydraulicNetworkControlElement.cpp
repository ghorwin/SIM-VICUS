#include "NANDRAD_HydraulicNetworkControlElement.h"

#include "NANDRAD_KeywordList.h"

#include <algorithm>

namespace NANDRAD {

void HydraulicNetworkControlElement::checkParameters(const std::vector<NANDRAD::Zone> &zones) {
	FUNCID(HydraulicNetworkControlElement::checkParameters);

	// NOTE: the check below is unecessary - should be ensured already through the "xml:required" specification!

	// this is a valid controller if a controlled property is chosen
	// so we need a valid controller type then
	if (m_controlledProperty != NUM_CP) {
		if (m_controllerType == NUM_CT)
			throw IBK::Exception("Missing ControllerType.", FUNC_ID);
	}

	try {
		// check individual configuations for different controller properties
		switch (m_controlledProperty) {
			case CP_TemperatureDifference: {
				m_setPoint.checkedValue("SetPoint", "K", "K", 0, false, std::numeric_limits<double>::max(), false, nullptr);
				// controller type must! be given
				if(m_controllerType == NUM_CT)
					throw IBK::Exception(IBK::FormatString("Missing attribute 'controllerType' for controlledProperty 'TemperatureDifference'!")
										 .arg(m_thermostatZoneID), FUNC_ID);
			} break;
			case CP_MassFlow: {
				m_setPoint.checkedValue("SetPoint", "kg/s", "kg/s", 0, false, std::numeric_limits<double>::max(), false, nullptr);
				// controller type must! be given
				if(m_controllerType == NUM_CT)
					throw IBK::Exception(IBK::FormatString("Missing attribute 'controllerType' for controlledProperty 'MassFlow'!")
										 .arg(m_thermostatZoneID), FUNC_ID);
			} break;
			case CP_ThermostatValue: {
				if(m_thermostatZoneID == NANDRAD::INVALID_ID) {
					throw IBK::Exception("Missing ThermostatZoneId!", FUNC_ID);
				}
				// check validity of thermostat zone
				std::vector<NANDRAD::Zone>::const_iterator zone_it = std::find(zones.begin(), zones.end(), m_thermostatZoneID);

				if (zone_it == zones.end())
					throw IBK::Exception(IBK::FormatString("Invalid/undefined zone with '%1' in ThermostatZoneId.")
										 .arg(m_thermostatZoneID), FUNC_ID);

				// controller type must! not be given
				if(m_controllerType != NUM_CT)
					throw IBK::Exception(IBK::FormatString("Attribute 'controllerType' is not allowed for controlledProperty 'ThermostatValue'!")
										 .arg(m_thermostatZoneID), FUNC_ID);
			} break;
			case NUM_CP:
				break;
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
			case NUM_CT: break;
		}
	}
	catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, "Missing/invalid parameters.", FUNC_ID);
	}
}

} // namespace NANDRAD
