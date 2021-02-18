#include "VICUS_ScheduleInterval.h"

#include <set>

#include <NANDRAD_Schedule.h>

namespace VICUS {

bool ScheduleInterval::isValid() const {
	// check that interval start day is in range 0..364
	if(m_intervalStartDay < 0 || m_intervalStartDay>364)
		return false;

	// check that the daily cycles define indeed all days

	if(m_dailyCycles.empty())
		return false;

	std::set<int>	allDayTypes, checkDayTypes;
	allDayTypes.insert(NANDRAD::Schedule::ST_MONDAY);
	allDayTypes.insert(NANDRAD::Schedule::ST_TUESDAY);
	allDayTypes.insert(NANDRAD::Schedule::ST_WEDNESDAY);
	allDayTypes.insert(NANDRAD::Schedule::ST_THURSDAY);
	allDayTypes.insert(NANDRAD::Schedule::ST_FRIDAY);
	allDayTypes.insert(NANDRAD::Schedule::ST_SATURDAY);
	allDayTypes.insert(NANDRAD::Schedule::ST_SUNDAY);
	allDayTypes.insert(NANDRAD::Schedule::ST_HOLIDAY);

	for(auto &  dC : m_dailyCycles){
		if(!dC.isValid())
			return false;

		for( int dT : dC.m_dayTypes)
			checkDayTypes.insert(dT);
	}

	if(allDayTypes != checkDayTypes)
		return false;

	return true;
}


} // namespace VICUS
