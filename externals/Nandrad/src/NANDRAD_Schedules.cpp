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

//bool Schedules::isDefault() const {
//	Schedules tmp;
//	tmp.initDefaults();
//	return (*this == tmp);
//}
#if 0
Schedules::DefaultParameters::DefaultParameters() {
	m_weekEndDays.insert( SD_SUNDAY );
	m_weekEndDays.insert( SD_SATURDAY );
}

void Schedules::DefaultParameters::readXML(const TiXmlElement * element) {
	const char * const FUNC_ID = "[DefaultParameters::readXML]";

	const TiXmlElement *c;
	try {
		// read sub-elements
		for (c = element->FirstChildElement(); c; c = c->NextSiblingElement()) {
			// determine data based on element name
			std::string cname = c->Value();

			if (cname == "WeekEndDays") {
				// clear defaults and old content if defined in project file
				m_weekEndDays.clear();

				std::list<std::string> dayList;
				IBK::explode(c->GetText(),dayList,',',true);

				// now read all weekend days
				for(std::list<std::string>::const_iterator it = dayList.begin();
					it != dayList.end(); ++it)
				{
					day_t day = (day_t) KeywordList::Enumeration("Schedules::day_t",*it);
					m_weekEndDays.insert(day);
				}

			}
			else if (cname == "Holidays" ) {

				// get attribute for format
				const TiXmlAttribute * attrib = TiXmlAttribute::attributeByName(c, "languageFormat");
				if (!attrib)
					throw IBK::Exception( IBK::FormatString( XML_READ_ERROR ).arg(c->Row()).arg(
						"Expected 'languageFormat' attribute in Holidays tag."
						), FUNC_ID);

				std::string attribString = attrib->Value();
				Date::LanguageID langID = (Date::LanguageID) KeywordList::Enumeration("Date::LanguageID", attribString);

				// clear defaults and old content if defined in project file
				m_holidays.clear();

				// determine contents of comma separated list
				std::string holidays = c->GetText();
				std::list< std::string > listOfHolidays;

				IBK::explode( holidays, listOfHolidays, ',', true );

				for (std::list<std::string>::const_iterator it = listOfHolidays.begin();
					it != listOfHolidays.end(); ++it)
				{
					Date date;
					try {
						date.decode( *it, langID );
						m_holidays.insert( date );
					}
					catch( IBK::Exception & ex ) {
						throw IBK::Exception(ex, IBK::FormatString( XML_READ_ERROR ).arg(c->Row()).arg(
							IBK::FormatString("Couldn't decode date string '%1' in holidays tag.").arg(*it)
							), FUNC_ID);

					}
				}
			}
			else {
				throw IBK::Exception( IBK::FormatString( XML_READ_ERROR ).arg(c->Row()).arg(
					IBK::FormatString("Unknown XML tag with name '%1' in DefaultParameters section.").arg(cname)
					), FUNC_ID);
			}
		}

	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading DefaultParameters data."), FUNC_ID);
	}
	catch (std::exception & ex) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading DefaultParameters data.").arg(ex.what()), FUNC_ID);
	}
}


void Schedules::DefaultParameters::writeXML(TiXmlElement * parent, bool detailedOutput) const {

	TiXmlElement * e = new TiXmlElement("Default");
	parent->LinkEndChild(e);

	// write validity date of schedule
	if(!m_weekEndDays.empty())  {

		if (detailedOutput)
			TiXmlComment::addComment(e, "List of all weekend days.");

		std::string days;
		for (std::set<day_t>::const_iterator it = m_weekEndDays.begin(); it != m_weekEndDays.end(); ++it) {
			if (it != m_weekEndDays.begin())
				days +=  ",";
			days += KeywordList::Keyword("Schedules::day_t",*it);
		}
		TiXmlElement::appendSingleAttributeElement(e,"WeekEndDays",nullptr,std::string(),days);
	}

	if (!m_holidays.empty()) {

		std::string holidays;
		if (detailedOutput)
			TiXmlComment::addComment(e, "List of all holidays.");


		for (std::set<Date>::const_iterator it = m_holidays.begin(); it != m_holidays.end(); ++it) {
			if (it != m_holidays.begin())
				holidays +=  ",";
			holidays += it->encode( NANDRAD::Date::L_EN );
		}
		TiXmlElement::appendSingleAttributeElement( e, "Holidays", "languageFormat",
													KeywordList::Keyword( "Date::LanguageID", Date::L_EN ), holidays);
	}
}


bool Schedules::DefaultParameters::operator!=(const Schedules::DefaultParameters & other) const {
	if (m_holidays != other.m_holidays) return true;
	if (m_weekEndDays != other.m_weekEndDays) return true;
	return false;
}

#endif
// *** Schedules ***

void Schedules::readXML(const TiXmlElement * element) {
	const char * const FUNC_ID = "[Schedules::readXML]";
#if 0
	// clear schedule groups and annual schedules
	m_scheduleGroups.clear();
	m_annualSchedules.m_parameters.clear();

	const TiXmlElement * c;
	try {
		// read sub-elements
		for ( c = element->FirstChildElement(); c; c = c->NextSiblingElement()) {
			// determine data based on element name
			std::string cname = c->Value();
			if (cname == "Default") {
				m_defaults.readXML(c);
			}
			else if (cname == "ScheduleGroup") {
				ScheduleGroup scheduleGroup;
				scheduleGroup.readXML(c);
				m_scheduleGroups.push_back(scheduleGroup);
			}
			else if (cname == "AnnualSchedules") {
				m_annualSchedules.readXML(c);
			}
			else {
				throw IBK::Exception( IBK::FormatString( XML_READ_ERROR ).arg(c->Row()).arg(
					IBK::FormatString("Unknown XML tag with name '%1' in Schedules section.").arg(cname)
					), FUNC_ID);
			}
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading Schedules data."), FUNC_ID);
	}
	catch (std::exception & ex) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading Schedules data.").arg(ex.what()), FUNC_ID);
	}

	if(m_scheduleGroups.empty() && m_annualSchedules.m_parameters.empty() )
		initDefaults();
#endif
}
// ----------------------------------------------------------------------------


void Schedules::writeXML(TiXmlElement * parent, bool detailedOutput) const {
#if 0
	if (isDefault())
		return;

	if (detailedOutput) {
		TiXmlComment::addComment(parent,
			"Schedules section defines user scenarios.");
		TiXmlComment::addComment(parent,
			"A scenario is defined by a single schedule group");
		TiXmlComment::addComment(parent,
			"containing different schedules for specific model components.");
	}

	TiXmlElement * e = new TiXmlElement("Schedules");
	parent->LinkEndChild(e);

	writeXML2(e, detailedOutput);
#endif
}
// ----------------------------------------------------------------------------


bool Schedules::operator!=(const Schedules & other) const {
//	if (m_defaults != other.m_defaults) return true;
	if (m_scheduleGroups != other.m_scheduleGroups) return true;
	if (m_annualSchedules != other.m_annualSchedules) return true;
	return false;
}
// ----------------------------------------------------------------------------


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

// ----------------------------------------------------------------------------
void Schedules::writeXML2(TiXmlElement * e, bool detailedOutput) const {
#if 0
	// write default parameter settings
	if (detailedOutput)
		TiXmlComment::addComment(e,"Defaults for all Schedules.");
//	m_defaults.writeXML(e, detailedOutput);

	if ( ! m_scheduleGroups.empty() ){

		if (detailedOutput) {
			// write all scehdule groups
			TiXmlComment::addComment(e,
				"A schedule group contains several schedules that define the course of.");
			TiXmlComment::addComment(e,
				"a physical quantity for a given time period. It includes a basic");
			TiXmlComment::addComment(e,
				"schedule for week days (all days), and then override schedules for");
			TiXmlComment::addComment(e,
				"weekends and holidays.");
		}

		for (std::vector<ScheduleGroup>::const_iterator it = m_scheduleGroups.begin();
			it != m_scheduleGroups.end(); ++it)
		{
			it->writeXML(e, detailedOutput);
		}

	}

	m_annualSchedules.writeXML(e, detailedOutput);
#endif
}
// ----------------------------------------------------------------------------


} // namespace NANDRAD

