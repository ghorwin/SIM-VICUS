#include "VICUS_DailyCycle.h"


#include <NANDRAD_Schedule.h>
namespace VICUS {

bool DailyCycle::isValid() const {

	if(m_dayTypes.empty() || m_dayTypes.size()> 7+1)	//Mo, ..., Sun + Holiday
		return false;	// day types not valid

	for( auto dt : m_dayTypes){
		if(!(dt == NANDRAD::Schedule::ST_MONDAY ||
				dt == NANDRAD::Schedule::ST_TUESDAY ||
				dt == NANDRAD::Schedule::ST_WEDNESDAY ||
				dt == NANDRAD::Schedule::ST_THURSDAY ||
				dt == NANDRAD::Schedule::ST_FRIDAY ||
				dt == NANDRAD::Schedule::ST_SATURDAY ||
				dt == NANDRAD::Schedule::ST_SUNDAY ||
				dt == NANDRAD::Schedule::ST_HOLIDAY))
			return false;	// day type is not valid
	}

	if(m_values.size() != m_timePoints.size() || m_values.empty())
		return false;	// empty values or size not same of values and timepoints

	if(m_timePoints[0] != 0)
		return false;	// start point is not 0

	// check time points vector and time point and value vectors have same size
	return true;
}


} // namespace VICUS
