#include "NANDRAD_Thermostat.h"

#include "NANDRAD_KeywordList.h"

namespace NANDRAD {

void Thermostat::checkParameters() const {

	m_para[P_TemperatureTolerance].checkedValue("TemperatureTolerance", "K", "K", 0, false, 10, true,
												"Temperature tolerance should be 0 K < tolerance < 10 K");


	// TODO Dirk
}


} // namespace NANDRAD

