/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
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


bool ScheduleInterval::usedAllDayTypes(bool withHolyday) const {
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


ScheduleInterval ScheduleInterval::multiply(const ScheduleInterval &other, unsigned int startDay) const {
	FUNCID(ScheduleInterval::multiply);
	ScheduleInterval schedInt;
	if (!isValid())
		IBK::Exception(IBK::FormatString("Invalid schedule interval."), FUNC_ID);

	if (!other.isValid())
		IBK::Exception(IBK::FormatString("Invalid schedule interval."), FUNC_ID);


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
	FUNCID(ScheduleInterval::multiply);
	ScheduleInterval schedInt;
	if (!isValid())
		IBK::Exception(IBK::FormatString("Invalid schedule interval."), FUNC_ID);


	schedInt = *this;

	for(unsigned int i=0; i<schedInt.m_dailyCycles.size(); ++i)
		schedInt.m_dailyCycles[i] = schedInt.m_dailyCycles[i] * val;

	return schedInt;
}


ScheduleInterval ScheduleInterval::add(const ScheduleInterval & other, unsigned int startDay) const {
	FUNCID(ScheduleInterval::add);
	ScheduleInterval schedInt;
	if (!isValid())
		IBK::Exception(IBK::FormatString("Invalid schedule interval."), FUNC_ID);


	if (!other.isValid())
		IBK::Exception(IBK::FormatString("Invalid schedule interval."), FUNC_ID);


	//add each daily cycle (dc) of the to dc with the second dc
	for(unsigned int i=0; i<m_dailyCycles.size(); ++i){
		for(unsigned int j=0; j<other.m_dailyCycles.size();++j){
			DailyCycle dc = m_dailyCycles[i] + other.m_dailyCycles[j];
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


ScheduleInterval ScheduleInterval::add(double val) const{
	FUNCID(ScheduleInterval::add);
	ScheduleInterval schedInt;
	if (!isValid())
		IBK::Exception(IBK::FormatString("Invalid schedule interval."), FUNC_ID);

	schedInt = *this;

	for(unsigned int i=0; i<schedInt.m_dailyCycles.size(); ++i)
		schedInt.m_dailyCycles[i] = schedInt.m_dailyCycles[i].add(val);

	return schedInt;
}


void ScheduleInterval::createConstScheduleInterval(double val){
	m_intervalStartDay=0;
	m_dailyCycles.push_back(DailyCycle());
	DailyCycle &dc = m_dailyCycles.back();
	dc.m_dayTypes.push_back(NANDRAD::Schedule::ST_MONDAY);
	dc.m_dayTypes.push_back(NANDRAD::Schedule::ST_TUESDAY);
	dc.m_dayTypes.push_back(NANDRAD::Schedule::ST_WEDNESDAY);
	dc.m_dayTypes.push_back(NANDRAD::Schedule::ST_THURSDAY);
	dc.m_dayTypes.push_back(NANDRAD::Schedule::ST_FRIDAY);
	dc.m_dayTypes.push_back(NANDRAD::Schedule::ST_SATURDAY);
	dc.m_dayTypes.push_back(NANDRAD::Schedule::ST_SUNDAY);
	dc.m_timePoints.push_back(0);
	dc.m_values.push_back(val);
}



void ScheduleInterval::createWeekDataVector(std::vector<double> &timepoints, std::vector<double> &data) const {

	std::vector<NANDRAD::Schedule::ScheduledDayType> dts{
		NANDRAD::Schedule::ST_MONDAY,
		NANDRAD::Schedule::ST_TUESDAY,
		NANDRAD::Schedule::ST_WEDNESDAY,
		NANDRAD::Schedule::ST_THURSDAY,
		NANDRAD::Schedule::ST_FRIDAY,
		NANDRAD::Schedule::ST_SATURDAY,
		NANDRAD::Schedule::ST_SUNDAY
	};
	for(NANDRAD::Schedule::ScheduledDayType dt1 : dts){
		for(unsigned int i = 0; i<m_dailyCycles.size(); ++i){
			const DailyCycle &dc = m_dailyCycles[i];
			for(unsigned int j = 0; j<dc.m_dayTypes.size(); ++j){
				NANDRAD::Schedule::ScheduledDayType dt = (NANDRAD::Schedule::ScheduledDayType)dc.m_dayTypes[j];
				if(dt == dt1){
					double addTime = 0;
					switch (dt1) {
						case NANDRAD::Schedule::ST_ALLDAYS:
						case NANDRAD::Schedule::ST_WEEKDAY:
						case NANDRAD::Schedule::ST_WEEKEND:
							//not used
						break;
						case NANDRAD::Schedule::ST_MONDAY:
							// is 0
						break;
						case NANDRAD::Schedule::ST_TUESDAY:
							addTime +=24*1;
						break;
						case NANDRAD::Schedule::ST_WEDNESDAY:
							addTime +=24*2;
						break;
						case NANDRAD::Schedule::ST_THURSDAY:
							addTime +=24*3;
						break;
						case NANDRAD::Schedule::ST_FRIDAY:
							addTime +=24*4;
						break;
						case NANDRAD::Schedule::ST_SATURDAY:
							addTime +=24*5;
						break;
						case NANDRAD::Schedule::ST_SUNDAY:
							addTime +=24*6;
						break;
						case NANDRAD::Schedule::ST_HOLIDAY:
							//not taken into account
						break;
						case NANDRAD::Schedule::NUM_ST:
						break;

					}
					//add timepoints
					for(double tp : dc.m_timePoints)
						timepoints.push_back(tp+addTime);
					//add data
					data.insert(data.end(), dc.m_values.begin(), dc.m_values.end());
				}
			}
		}
	}

	// special handling for empty daily cycles
	if (timepoints.empty()) {
		timepoints.push_back(0);
		data.push_back(0);
	}

	// now the data vector contains all data for a week starting at monday
}

void ScheduleInterval::calculateMinMax(double &min, double &max) const {
	min = std::numeric_limits<double>::max();
	max = std::numeric_limits<double>::min();
	for(unsigned int i=0; i<m_dailyCycles.size(); ++i) {
		double minVal;
		double maxVal;
		m_dailyCycles[i].calculateMinMax(minVal, maxVal);
		min = std::min<double>(min, minVal);
		max = std::max<double>(max, maxVal);
	}
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
