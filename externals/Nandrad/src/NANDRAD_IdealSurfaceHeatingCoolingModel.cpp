#include "NANDRAD_IdealSurfaceHeatingCoolingModel.h"

#include <algorithm>

#include "NANDRAD_KeywordList.h"

namespace NANDRAD {

void IdealSurfaceHeatingCoolingModel::checkParameters(const std::vector<NANDRAD::Zone> & zones) {
	FUNCID(IdealSurfaceHeatingCoolingModel::checkParameters);

	// all models require an object list with indication of construction that this model applies to
	if (m_constructionObjectList.empty())
		throw IBK::Exception(IBK::FormatString("Missing 'ConstructionObjectList' parameter."), FUNC_ID);

	// either maximum heating or cooling power must be given
	if (m_para[P_MaxHeatingPowerPerArea].name.empty() && m_para[P_MaxCoolingPowerPerArea].name.empty())
		throw IBK::Exception("Either maximum heating power or maximum cooling power must be defined!", FUNC_ID);

	// if maximum heating power is defined than check value
	if (!m_para[P_MaxHeatingPowerPerArea].name.empty()) {
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
	if (!m_para[P_MaxCoolingPowerPerArea].name.empty()) {
		m_para[P_MaxCoolingPowerPerArea].checkedValue("MaxCoolingPowerPerArea", "W/m2", "W/m2",
												   0, true,
												   std::numeric_limits<double>::max(), true,
												   "Maximum for cooling power must be >= 0 W/m2.");
	}
	// otherwise fill with 0
	else {
		m_para[P_MaxCoolingPowerPerArea].set("MaxCoolingPowerPerArea", 0, IBK::Unit("W/m2"));
	}

	// resolve thermostat zone
	std::vector<NANDRAD::Zone>::const_iterator zone_it = std::find(zones.begin(), zones.end(), m_thermostatZoneID);

	if (zone_it == zones.end())
		throw IBK::Exception(IBK::FormatString("Invalid/undefined zone with '%1' in ThermostatZoneId.")
							 .arg(m_thermostatZoneID), FUNC_ID);

}


} // namespace NANDRAD

