#include "NANDRAD_Controller.h"

#include "NANDRAD_KeywordList.h"


namespace NANDRAD {

void Controller::checkParameters() {
	FUNCID(Controller::checkParameters);

	try {
		// decide which parameters are needed
		switch (m_modelType) {
			case MT_DigitalDirect:
			case MT_DigitalHysteresis:
				break;
			case MT_PController: {
				m_para[P_Kp].checkedValue("Kp", "---", "---", 0, false, std::numeric_limits<double>::max(), true, nullptr);
			} break;
			case MT_PIController: {
				m_para[P_Kp].checkedValue("Kp", "---", "---", 0, false, std::numeric_limits<double>::max(), true, nullptr);
				m_para[P_Ki].checkedValue("Ki", "---", "---", 0, false, std::numeric_limits<double>::max(), true, nullptr);
			} break;
			default: break;
		}
	}
	catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Missing/invalid parameters for controller %1.")
				 .arg(KeywordList::Keyword("Controller::Type", m_modelType)),
				 FUNC_ID);
	}
}

}
