#include "VICUS_ScheduleInterval.h"

#include <set>

#include <NANDRAD_Schedule.h>

namespace VICUS {

bool ScheduleInterval::isValid() const {
	// check that interval start day is in range 0..364
	if (m_intervalStartDay>364)
		return false;

	// check that the daily cycles define indeed all days

	if(m_dailyCycles.empty())
		return false;

	// we need to check two things:
	// - all day times must be parametrized
	// - there must not be duplicate day type definitions

	std::set<int>	parametrizedDayTypes;
	// process all daily cycles
	for (const DailyCycle & dC : m_dailyCycles) {
		// check if DailyCycle itself is correct (contains correct day types)
		if (!dC.isValid())
			return false;

		// check if any of the parametrized day types has previously been defined
		// and remember day typers

		for ( int dT : dC.m_dayTypes) {
			if (parametrizedDayTypes.find(dT) != parametrizedDayTypes.end())
				return false; // already in set, duplicate!

			parametrizedDayTypes.insert(dT);
		}
	}
	// Holidays are optional, so we just insert this day type
	parametrizedDayTypes.insert(NANDRAD::Schedule::ST_HOLIDAY);

	// now remove all days that we require
	std::set<int>	allDayTypes;
	allDayTypes.insert(NANDRAD::Schedule::ST_MONDAY);
	allDayTypes.insert(NANDRAD::Schedule::ST_TUESDAY);
	allDayTypes.insert(NANDRAD::Schedule::ST_WEDNESDAY);
	allDayTypes.insert(NANDRAD::Schedule::ST_THURSDAY);
	allDayTypes.insert(NANDRAD::Schedule::ST_FRIDAY);
	allDayTypes.insert(NANDRAD::Schedule::ST_SATURDAY);
	allDayTypes.insert(NANDRAD::Schedule::ST_SUNDAY);
	allDayTypes.insert(NANDRAD::Schedule::ST_HOLIDAY);

	// both sets must contain all required day types
	if (allDayTypes != parametrizedDayTypes)
		return false;

	return true;
}


} // namespace VICUS
