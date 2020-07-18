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
#include <IBK_messages.h>

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
	FUNCID(Schedules::readXML);

	// reading elements
	const TiXmlElement * c = element->FirstChildElement();
	while (c) {
		const std::string & cName = c->ValueStr();
		if (cName == "Holidays") {
			const TiXmlElement * c2 = c->FirstChildElement();
			while (c2) {
				const std::string & c2Name = c2->ValueStr();
				if (c2Name != "IBK:Time")
					throw IBK::Exception(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), FUNC_ID);

				IBK::Time t = IBK::Time::fromDateTimeFormat(c2->GetText());
				if (!t.isValid())
					throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(c2->Row())
										 .arg("Invalid date/time format '"+std::string(c2->GetText())+"'"), FUNC_ID);
				if (m_holidays.find(t) != m_holidays.end())
					throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(c2->Row())
										 .arg("Duplicate holiday date '"+std::string(c2->GetText())+"'"), FUNC_ID);
				m_holidays.insert(t);

				c2 = c2->NextSiblingElement();
			}

		}
		else if (cName == "WeekEndDays") {
			std::string days = c->GetText();
			// split days at ,
			std::vector<std::string> dayvec;
			IBK::explode(days, dayvec, ",", IBK::EF_TrimTokens);
			// convert all days into enums
			for (const std::string & d : dayvec) {
				try {
					day_t dt = (day_t)KeywordList::Enumeration("Schedules::day_t", d);
					m_weekEndDays.insert(dt);
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception(ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
										 .arg("Invalid day name in 'WeekEndDays' tag."), FUNC_ID);
				}
			}
		}
		else if (cName == "ScheduleGroups") {
			const TiXmlElement * c2 = c->FirstChildElement();
			while (c2) {
				const std::string & c2Name = c2->ValueStr();
				if (c2Name != "ScheduleGroup")
					throw IBK::Exception(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), FUNC_ID);

				const TiXmlAttribute * attrib = TiXmlAttribute::attributeByName(c2, "objectList");
				if (attrib == nullptr)
					throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(c2->Row()).arg(
						"Missing required 'objectList' attribute."), FUNC_ID);

				std::string objectListName = attrib->ValueStr();
				// ensure that we do not have duplicate definitions
				if (m_scheduleGroups.find(objectListName) != m_scheduleGroups.end())
					throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(c2->Row()).arg(
						"ObjectList '"+objectListName+"' used for multiple ScheduleGroup elements."), FUNC_ID);

				std::vector<Schedule> schedules;
				// now read all the schedule subtags

				const TiXmlElement * c3 = c2->FirstChildElement();
				while (c3) {
					const std::string & c3Name = c3->ValueStr();
					if (c3Name != "Schedule")
						throw IBK::Exception(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c3Name).arg(c3->Row()), FUNC_ID);

					Schedule s;
					try {
						s.readXML(c3);
					} catch (IBK::Exception & ex) {
						throw IBK::Exception(ex, IBK::FormatString(XML_READ_ERROR).arg(c3->Row()).arg(
							"Error reading 'Schedule' element."), FUNC_ID);
					}
					schedules.push_back(s);

					c3 = c3->NextSiblingElement();
				}

				m_scheduleGroups[objectListName] = schedules;

				c2 = c2->NextSiblingElement();
			}
		}
		else if (cName == "AnnualSchedules") {
			const TiXmlElement * c2 = c->FirstChildElement();
			while (c2) {
				const std::string & c2Name = c2->ValueStr();
				if (c2Name != "IBK:LinearSpline")
					throw IBK::Exception(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), FUNC_ID);
				try {
					NANDRAD::LinearSplineParameter spl;
					spl.readXML(c2);
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception(ex, IBK::FormatString(XML_READ_ERROR).arg(c2->Row())
										 .arg("Invalid data in 'IBK:LinearSpline' tag."), FUNC_ID);
				}

				c2 = c2->NextSiblingElement();
			}
		}
		else {
			IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
		}
		c = c->NextSiblingElement();
	}
}


TiXmlElement * Schedules::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("Schedules");
	parent->LinkEndChild(e);

	if (!m_holidays.empty()) {
		TiXmlElement * c = new TiXmlElement("Holidays");
		e->LinkEndChild(c);
		// encode days
		for (IBK::Time t : m_holidays)
			TiXmlElement::appendSingleAttributeElement(c, "IBK:Time", nullptr, std::string(), t.toDateTimeFormat());
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

	// now write schedules
	if (!m_scheduleGroups.empty()) {
		TiXmlElement * c = new TiXmlElement("ScheduleGroups");
		e->LinkEndChild(c);
		for (const std::pair<const std::string, std::vector<Schedule> > & svec : m_scheduleGroups) {
			// create tags like
			// <ScheduleGroup objectList="objectListName">
			//   <Schedule>...</Schedule>
			// </ScheduleGroup>
			TiXmlElement * g = new TiXmlElement("ScheduleGroup");
			c->LinkEndChild(g);
			g->SetAttribute("objectList", svec.first);
			for (const Schedule & s : svec.second)
				s.writeXML(g);
		}
	}

	// now write schedules
	if (!m_annualSchedules.empty()) {
		TiXmlElement * c = new TiXmlElement("AnnualSchedules");
		e->LinkEndChild(c);
		for (const NANDRAD::LinearSplineParameter & s : m_annualSchedules)
			s.writeXML(e);
	}

	return nullptr;
}


bool Schedules::operator!=(const Schedules & other) const {
	if (m_holidays != other.m_holidays) return true;
	if (m_weekEndDays != other.m_weekEndDays) return true;
	if (m_scheduleGroups != other.m_scheduleGroups) return true;
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

