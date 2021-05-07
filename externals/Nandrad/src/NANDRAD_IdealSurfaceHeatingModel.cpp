#include "NANDRAD_IdealSurfaceHeatingModel.h"

#include "NANDRAD_KeywordList.h"

namespace NANDRAD {

void IdealSurfaceHeatingModel::checkParameters() const {
	m_para[P_MaxHeatingPowerPerArea].checkedValue("MaxHeatingPowerPerArea", "W/m2", "W/m2",
											   0, true,
											   std::numeric_limits<double>::max(), true,
											   "Maximum for heating power must be >= 0 W/m2.");
}


} // namespace NANDRAD

