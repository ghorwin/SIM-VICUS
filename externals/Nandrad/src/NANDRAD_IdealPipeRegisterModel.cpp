#include "NANDRAD_IdealPipeRegisterModel.h"

#include "NANDRAD_KeywordList.h"
#include "NANDRAD_Zone.h"

#include <algorithm>

namespace NANDRAD {

void IdealPipeRegisterModel::checkParameters(const std::vector<NANDRAD::Zone> &zones) const {
	FUNCID(IdealPipeRegisterModel::checkParameters);
	// check parameters
	m_para[P_MaxMassFlow].checkedValue("MaxMassFlow", "kg/s", "kg/s",
											   0, true,
											   std::numeric_limits<double>::max(), true,
											   "Maximum mass flow must be >= 0 kg/s.");
	// decide how to recieve supply temperature
	if(m_modelType == MT_Constant) {
		m_para[P_SupplyTemperature].checkedValue("SupplyTemperature", "C", "K",
												   0.0, true,
												   std::numeric_limits<double>::max(), true,
												   "Supply temperature must be >= 0 K.");
	}

	// check validity of thermostat zone
	std::vector<NANDRAD::Zone>::const_iterator zone_it = std::find(zones.begin(),
																  zones.end(),
																  m_thermostatZoneID);

	if (zone_it == zones.end())
		throw IBK::Exception(IBK::FormatString("Invalid/undefined zone with '%1' in ThermostatZoneId.")
							 .arg(m_thermostatZoneID), FUNC_ID);

}


} // namespace NANDRAD

