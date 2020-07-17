/*	The NANDRAD data model library.
Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
St. Vogelsang	<stefan.vogelsang -[at]- tu-dresden.de>
All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
*/

#include "NANDRAD_Schedules.h"
#include "NANDRAD_KeywordList.h"
#include "NANDRAD_Utilities.h"
#include "NANDRAD_Constants.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>

#include <tinyxml.h>

namespace NANDRAD {
#if 0
void Schedules::initDefaults() {
	// add a new schedule group with all parameters
	// for a default space type
	ScheduleGroup scheduleGroup;
	// define an alldays schedule
	Schedule schedule;
	schedule.m_type = NANDRAD::Schedule::ST_ALLDAYS;
	// define a daily cycle
	DailyCycle dailyCycle;
	// define an interval for daily cycle
	Interval interval;
	// define parameters for heating setpoint
	interval.m_genericParaConst["HeatingSetPointTemperature"] =
		IBK::Parameter("HeatingSetPointTemperature", 21, "C");
	// add to daily cycle
	dailyCycle.m_intervals.push_back(interval);
	// add to schedule
	schedule.m_dailyCycles.push_back(dailyCycle);
	// add to space type group "Default"
	std::map<Schedule::type_t, Schedule> scheduleMap;
	scheduleMap[NANDRAD::Schedule::ST_ALLDAYS] = schedule;
	scheduleGroup.m_spaceTypeGroups["Default"] = scheduleMap;
	// add to schedules
	m_scheduleGroups.push_back(scheduleGroup);
}
#endif

void Schedules::readXML(const TiXmlElement * element) {
}

TiXmlElement * Schedules::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("Schedules");
	parent->LinkEndChild(e);

	if (!m_holidays.empty()) {
		TiXmlElement * c = new TiXmlElement("Holidays");
		e->LinkEndChild(c);
		// encode days
		for (IBK::Time t : m_holidays)
			TiXmlElement::appendSingleAttributeElement(c, "IBK::Time", nullptr, std::string(), t.toDateTimeFormat());
	}

	if (!m_weekEndDays.empty()) {
		TiXmlElement * c = new TiXmlElement("WeekEndDays");
		e->LinkEndChild(c);
		// encode days
		std::string days;
		for (day_t t : m_weekEndDays)
			days += std::string(",") + KeywordList::Keyword("Schedules::day_t", t);
		TiXmlText * text = new TiXmlText( days.substr(1) ); // Mind: remove leading , from string
		c->LinkEndChild(text);
	}



	return nullptr;
}


bool Schedules::operator!=(const Schedules & other) const {
	if (m_holidays != other.m_holidays) return true;
	if (m_weekEndDays != other.m_weekEndDays) return true;
	if (m_schedules != other.m_schedules) return true;
	if (m_annualSchedules != other.m_annualSchedules) return true;
	return false;
}
// ----------------------------------------------------------------------------
#if 0

void Schedules::parameterList(std::map< std::string, bool > & parameterIDNames) const {

	parameterIDNames.clear();

	// first read all annual schedules
	std::map<std::string, AnnualSchedules::LinearSplineParameterMap>::const_iterator it = m_annualSchedules.m_parameters.begin();

	for ( ; it != m_annualSchedules.m_parameters.end(); ++it ) {

		AnnualSchedules::LinearSplineParameterMap::const_iterator mit = it->second.begin();
		for ( ; mit != it->second.end(); ++mit ) {

			std::string name = mit->first; //it->first + ":" +
			//parameterIDNames.push_back( std::pair<bool, std::string>(true, name ) );
			parameterIDNames[name] = true;
		}
	}

	for ( unsigned int i=0; i<m_scheduleGroups.size(); ++i ) {

		std::map<std::string, ScheduleGroup::ScheduleMap>::const_iterator it = m_scheduleGroups[i].m_spaceTypeGroups.begin();
		for ( ; it != m_scheduleGroups[i].m_spaceTypeGroups.end(); ++it ) {

			std::map<Schedule::type_t, Schedule>::const_iterator mit = it->second.begin();

			for ( ; mit != it->second.end(); ++mit ) {

				std::string name = KeywordList::Keyword( "Schedule::type_t", mit->first); //it->first + ":" +
				//parameterIDNames.push_back( std::pair<bool, std::string>(false, name ) );
				parameterIDNames[name] = false;
			}
		}
	}

	/// \todo Anne, ausprogrammieren für Schedulegroups

}
// ----------------------------------------------------------------------------


void Schedules::parameterValues(const std::string & parameterIDName, NANDRAD::LinearSplineParameter & values) const {

	values.m_values.clear();

	//unsigned pos = parameterIDName.find(":");

	//std::string spacetype = parameterIDName.substr(0,pos);
	std::string parameter = parameterIDName; //.substr(pos+1);

	std::map<std::string, AnnualSchedules::LinearSplineParameterMap>::const_iterator it = m_annualSchedules.m_parameters.begin(); //.find( spacetype );
	if ( it != m_annualSchedules.m_parameters.end() ) {

		AnnualSchedules::LinearSplineParameterMap::const_iterator mit = it->second.find( parameter );
		if ( mit != it->second.end() ) {

			values = mit->second;
		}
	}

	/// \todo Anne, ausprogrammieren für Schedulegroups

}
// ----------------------------------------------------------------------------

void Schedules::scheduleList(std::map< std::string, bool > & scheduleNames) const {

	scheduleNames.clear();

	// first read all annual schedules
	std::map<std::string, AnnualSchedules::LinearSplineParameterMap>::const_iterator it = m_annualSchedules.m_parameters.begin();

	for ( ; it != m_annualSchedules.m_parameters.end(); ++it ) {

		scheduleNames[it->first] = true;
	}

	for ( unsigned int i=0; i<m_scheduleGroups.size(); ++i ) {

		std::map<std::string, ScheduleGroup::ScheduleMap>::const_iterator it = m_scheduleGroups[i].m_spaceTypeGroups.begin();
		for ( ; it != m_scheduleGroups[i].m_spaceTypeGroups.end(); ++it ) {

			scheduleNames[it->first] = false;
		}
	}

}
#endif


} // namespace NANDRAD

