#include "NANDRAD_ControlElement.h"

#include <algorithm>

#include "NANDRAD_Controller.h"


namespace NANDRAD {


void ControlElement::checkParameters(const std::vector<Controller> &controllers) {
	FUNCID("ControlElement::checkParameters");

	if (m_controlType != NUM_CT){
		auto it = std::find(controllers.begin(), controllers.end(), m_controllerId);
		if (it == controllers.end())
			throw IBK::Exception(IBK::FormatString("Controller with id #%1 does not exist.")
								 .arg(m_controllerId), FUNC_ID);

		// assign controller pointer
		m_controller = &(*it);

		switch (m_controlType) {
			case CT_ControlTemperatureDifference:
				m_setPoint.checkedValue("SetPoint", "K", "K", 0, false, std::numeric_limits<double>::max(), false, nullptr);
				break;
			case CT_ControlMassFlow:
				m_setPoint.checkedValue("SetPoint", "kg/s", "kg/s", 0, false, std::numeric_limits<double>::max(), false, nullptr);
				break;
			case NUM_CT:
				break;
		}

	}


	// TODO: other checks
}


} // namespace NANDRAD
