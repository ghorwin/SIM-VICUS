#include "NANDRAD_IdealSurfaceHeatingCoolingModel.h"

#include "NANDRAD_KeywordList.h"

namespace NANDRAD {

void IdealSurfaceHeatingCoolingModel::checkParameters() {
	FUNCID(IdealSurfaceHeatingCoolingModel::checkParameters);

	// either maximum heating or cooling power must be given
	if(m_para[P_MaxHeatingPowerPerArea].name.empty() &&
	   m_para[P_MaxCoolingPowerPerArea].name.empty()) {
		throw IBK::Exception("Either maximum heating power or maximum cooling power must be defined!",
							 FUNC_ID);
	}

	// if maximum heating power is defined than check value
	if(!m_para[P_MaxHeatingPowerPerArea].name.empty()) {
		m_para[P_MaxHeatingPowerPerArea].checkedValue("MaxHeatingPowerPerArea", "W/m2", "W/m2",
												   0, true,
												   std::numeric_limits<double>::max(), true,
												   "Maximum for heating power must be >= 0 W/m2.");
	}
	// otherwise fill with 0
	else {
		m_para[P_MaxHeatingPowerPerArea].set("MaxHeatingPowerPerArea", 0, IBK::Unit("W/m2"));
	}
	// if maximum cooling power is defined than check value
	if(!m_para[P_MaxHeatingPowerPerArea].name.empty()) {
		m_para[P_MaxCoolingPowerPerArea].checkedValue("MaxCoolingPowerPerArea", "W/m2", "W/m2",
												   0, true,
												   std::numeric_limits<double>::max(), true,
												   "Maximum for cooling power must be >= 0 W/m2.");
	}
	// otherwise fill with 0
	else
	{
		m_para[P_MaxCoolingPowerPerArea].set("MaxCoolingPowerPerArea", 0, IBK::Unit("W/m2"));
	}
}


} // namespace NANDRAD

