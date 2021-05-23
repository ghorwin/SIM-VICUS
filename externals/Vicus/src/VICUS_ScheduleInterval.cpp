/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  ... all the others from the SIM-VICUS team ... :-)

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

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

bool ScheduleInterval::usedAllDayTypes(bool withHolyday) const
{
	//set for all day types
	std::set<int> allDayTypes;

	for(const DailyCycle & dc : m_dailyCycles){
		//save size of set for compare later
		unsigned int dtSizeBefore = allDayTypes.size();
		//add all current day types to set
		for(int dt : dc.m_dayTypes)
			allDayTypes.insert(dt);
		//check if a day type was already in this set
		if(dtSizeBefore == allDayTypes.size() &&dc.m_dayTypes.size()>0){

		}
	}

	//create a set of all valid day types
	std::set<int> allValidDayTypes;
	allValidDayTypes.insert(NANDRAD::Schedule::ST_MONDAY);
	allValidDayTypes.insert(NANDRAD::Schedule::ST_TUESDAY);
	allValidDayTypes.insert(NANDRAD::Schedule::ST_WEDNESDAY);
	allValidDayTypes.insert(NANDRAD::Schedule::ST_THURSDAY);
	allValidDayTypes.insert(NANDRAD::Schedule::ST_FRIDAY);
	allValidDayTypes.insert(NANDRAD::Schedule::ST_SATURDAY);
	allValidDayTypes.insert(NANDRAD::Schedule::ST_SUNDAY);
	if(withHolyday)
		allValidDayTypes.insert(NANDRAD::Schedule::ST_HOLIDAY);

	//compare the two sets
	return allDayTypes == allValidDayTypes;

}

std::set<int> ScheduleInterval::freeDayTypes(){
	std::set<int> allValidDayTypes;
	allValidDayTypes.insert(NANDRAD::Schedule::ST_MONDAY);
	allValidDayTypes.insert(NANDRAD::Schedule::ST_TUESDAY);
	allValidDayTypes.insert(NANDRAD::Schedule::ST_WEDNESDAY);
	allValidDayTypes.insert(NANDRAD::Schedule::ST_THURSDAY);
	allValidDayTypes.insert(NANDRAD::Schedule::ST_FRIDAY);
	allValidDayTypes.insert(NANDRAD::Schedule::ST_SATURDAY);
	allValidDayTypes.insert(NANDRAD::Schedule::ST_SUNDAY);
	allValidDayTypes.insert(NANDRAD::Schedule::ST_HOLIDAY);

	for(const DailyCycle & dc : m_dailyCycles){
		//delete all current day types in set
		for(int dt : dc.m_dayTypes){
			if(allValidDayTypes.find(dt) != allValidDayTypes.end())
				allValidDayTypes.erase(dt);
		}
	}
	return allValidDayTypes;
}

ScheduleInterval ScheduleInterval::multiply(const ScheduleInterval &other, unsigned int startDay) const
{
	ScheduleInterval schedInt;
	if(!isValid()){
		//Schedule interval '%1' with (id=%2) is not valid.
		return schedInt;
	}

	if(!other.isValid()){
		//Schedule interval '%1' with (id=%2) is not valid.
		return schedInt;
	}


	//multiply each daily cycle (dc) of the first dc with the second dc
	for(unsigned int i=0; i<m_dailyCycles.size(); ++i){
		for(unsigned int j=0; j<other.m_dailyCycles.size();++j){
			DailyCycle dc = m_dailyCycles[i] * other.m_dailyCycles[j];
			if(dc != DailyCycle())
				schedInt.m_dailyCycles.push_back(dc);
		}
	}
	if(schedInt.m_dailyCycles.empty()){
		return ScheduleInterval();
	}

	schedInt.m_displayName = m_displayName;
	schedInt.m_intervalStartDay = startDay;

	return schedInt;
}

ScheduleInterval ScheduleInterval::multiply(double val) const{
	FUNCID("ScheduleInterval::multiply");
	ScheduleInterval schedInt;
	if(!isValid()){
		//Schedule interval '%1' with (id=%2) is not valid.
		return schedInt;
	}

	if(val<0)
		IBK::Exception(IBK::FormatString("Multiply negative values to a schedule interval is not allowed."), FUNC_ID);

	schedInt = *this;

	for(unsigned int i=0; i<schedInt.m_dailyCycles.size(); ++i)
		schedInt.m_dailyCycles[i] = schedInt.m_dailyCycles[i] * val;

	return schedInt;

}

bool ScheduleInterval::operator!=(const ScheduleInterval &other) const {
	if(m_displayName != other.m_displayName ||
			m_intervalStartDay != other.m_intervalStartDay)
		return true;

	if(m_dailyCycles != other.m_dailyCycles)
		return true;

	return false;
}


} // namespace VICUS
