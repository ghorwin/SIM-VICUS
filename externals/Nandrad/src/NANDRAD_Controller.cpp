#include "NANDRAD_Controller.h"

#include "NANDRAD_KeywordList.h"


namespace NANDRAD {


Controller::Controller()
{

}

void Controller::checkParameters()
{
	FUNCID(Controller::checkParameters);

	try {
		// decide which parameters are needed
		switch (m_type) {
			case T_DigitalDirect:
			case T_DigitalHysteresis:
				break;
			case T_PController: {
				m_par[P_Kp].checkedValue("Kp", "---", "---", 0, false, std::numeric_limits<double>::max(), true, nullptr);
			} break;
			case T_PIController: {
				m_par[P_Kp].checkedValue("Kp", "---", "---", 0, false, std::numeric_limits<double>::max(), true, nullptr);
				m_par[P_Ki].checkedValue("Ki", "---", "---", 0, false, std::numeric_limits<double>::max(), true, nullptr);
			} break;
			default: break;
		}
	}
	catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Missing/invalid parameters for controller %1.")
				 .arg(KeywordList::Keyword("Controller::Type", m_type)),
				 FUNC_ID);
	}
}

}
