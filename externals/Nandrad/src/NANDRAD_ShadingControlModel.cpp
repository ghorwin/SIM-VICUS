#include "NANDRAD_ShadingControlModel.h"

#include "NANDRAD_Sensor.h"

#include <NANDRAD_KeywordList.h>

#include <algorithm>

namespace NANDRAD {

void ShadingControlModel::checkParameters(const std::vector<Sensor> &sensors)
{
	FUNCID(ShadingControlModel::checkParameters);

	// retrieve network component
	std::vector<Sensor>::const_iterator sit =
			std::find(sensors.begin(), sensors.end(), m_sensorID);
	if (sit == sensors.end()) {
		throw IBK::Exception(IBK::FormatString("Sensor with id #%1 does not exist.")
							 .arg(m_sensorID), FUNC_ID);
	}
	// set reference
	m_sensor = &(*sit);

}


} // namespace NANDRAD
