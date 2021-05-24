#include "NANDRAD_HydraulicNetworkControlElement.h"


#include "NANDRAD_KeywordList.h"


namespace NANDRAD {


void HydraulicNetworkControlElement::checkParameters() {
	FUNCID(HydraulicNetworkControlElement::checkParameters);

	// this is a valid controller if a controlled property is chosen
	// so we need a valid controller type then
	if (m_controlledProperty != NUM_CP){
		if (m_controllerType == NUM_CT)
			throw IBK::Exception(IBK::FormatString("Missing ControllerType for ControlElement '%1'")
							 .arg(m_id),FUNC_ID);
	}

	// check set point
	switch (m_controlledProperty) {
		case CP_TemperatureDifference:
			m_setPoint.checkedValue("SetPoint", "K", "K", 0, false, std::numeric_limits<double>::max(), false, nullptr);
			break;
		case CP_MassFlow:
			m_setPoint.checkedValue("SetPoint", "kg/s", "kg/s", 0, false, std::numeric_limits<double>::max(), false, nullptr);
			break;
		case NUM_CP:
			break;
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
			throw IBK::Exception(ex, IBK::FormatString("Missing/invalid parameters for ControlElement '%1'")
				 .arg(m_id),FUNC_ID);
	}


}


} // namespace NANDRAD
