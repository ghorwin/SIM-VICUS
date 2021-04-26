#include "NANDRAD_IdealHeatingCoolingModel.h"

#include "NANDRAD_KeywordList.h"

namespace NANDRAD {

void IdealHeatingCoolingModel::checkParameters() const {
	m_para[P_MaxHeatingPowerPerArea].checkedValue("MaxHeatingPowerPerArea", "W/m2K", "W/m2K",
											   0, true,
											   std::numeric_limits<double>::max(), true,
											   "Maximum for heating power must be >= 0 W/m2K.");
	m_para[P_MaxCoolingPowerPerArea].checkedValue("MaxCoolingPowerPerArea", "W/m2K", "W/m2K",
											   0, true,
											   std::numeric_limits<double>::max(), true,
											   "Maximum for cooling power must be >= 0 W/m2K.");
}


} // namespace NANDRAD

