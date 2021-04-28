#include "NANDRAD_Thermostat.h"

#include "NANDRAD_KeywordList.h"

namespace NANDRAD {

void Thermostat::checkParameters() const {

	switch (m_controllerType) {
		case NANDRAD::Thermostat::NUM_CT:
		case NANDRAD::Thermostat::CT_PController:
			// controller needs temperature tolerance
			m_para[P_TemperatureTolerance].checkedValue("TemperatureTolerance", "K", "K", 0, false, 10, true,
														"Temperature tolerance should be 0 K < tolerance < 10 K");
		break;

		case NANDRAD::Thermostat::CT_DigitalController:
			m_para[P_TemperatureBand].checkedValue("TemperatureBand", "K", "K", 0, false, 10, true,
														"Temperature band should be 0 K < band < 10 K");

		break;
	}

}


} // namespace NANDRAD

