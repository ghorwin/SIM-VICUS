#include "NANDRAD_IdealPipeRegisterModel.h"

#include "NANDRAD_KeywordList.h"

namespace NANDRAD {

void IdealPipeRegisterModel::checkParameters() const {
	// check parameters
	m_para[P_SupplyTemperature].checkedValue("SupplyTemperature", "C", "K",
											   0.0, true,
											   std::numeric_limits<double>::max(), true,
											   "Supply temperature must be >= 0 K.");
	m_para[P_MaxMassFlow].checkedValue("MaxMassFlow", "kg/s", "kg/s",
											   0, true,
											   std::numeric_limits<double>::max(), true,
											   "Maximum mass flow must be >= 0 kg/s.");

}


} // namespace NANDRAD

