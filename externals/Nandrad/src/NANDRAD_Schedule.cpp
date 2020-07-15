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

#include "NANDRAD_Schedule.h"
#include "NANDRAD_KeywordList.h"
#include "NANDRAD_Constants.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <IBK_messages.h>

#include <tinyxml.h>

namespace NANDRAD {

Schedule::Schedule() :
	m_type(NUM_ST)
{
}


void Schedule::readXML(const TiXmlElement * element) {

	const char * const FUNC_ID = "[Schedule::readXML]";

	// read attributes
	const TiXmlAttribute * attrib = TiXmlAttribute::attributeByName(element, "type");
	if (!attrib)
		throw IBK::Exception( IBK::FormatString( XML_READ_ERROR ).arg(element->Row()).arg(
			"Expected 'type' attribute in Schedule tag."
			), FUNC_ID);
	try {
		m_type = (type_t) KeywordList::Enumeration("Schedule::type_t", attrib->Value());
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex,  IBK::FormatString( XML_READ_ERROR ).arg(element->Row()).arg(
			IBK::FormatString("Invalid name '%1' for 'type' attribute in Schedule tag.").arg(attrib->Value())
			), FUNC_ID);
	}

	// now read data
	const TiXmlElement * c;
	try {
		// read sub-elements
		for (c = element->FirstChildElement(); c; c = c->NextSiblingElement()) {
			// determine data based on element name
			std::string cname = c->Value();
			if (cname == "DailyCycle") {
				DailyCycle dailyCycle;
				dailyCycle.readXML(c);
				m_dailyCycles.push_back(dailyCycle);
			}
			else {
				throw IBK::Exception( IBK::FormatString( XML_READ_ERROR ).arg(c->Row()).arg(
					IBK::FormatString("Unknown XML tag with name '%1' in Schedule section.").arg(cname)
					), FUNC_ID);
			}
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading Schedule data."), FUNC_ID);
	}
	catch (std::exception & ex) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading Schedule data.").arg(ex.what()), FUNC_ID);
	}
}


void Schedule::writeXML(TiXmlElement * parent) const {

	TiXmlElement * e = new TiXmlElement("Schedule");
	parent->LinkEndChild(e);

	e->SetAttribute("type", KeywordList::Keyword("Schedule::type_t",m_type));

	for (std::vector<DailyCycle>::const_iterator it = m_dailyCycles.begin();
		it != m_dailyCycles.end(); ++it)
	{
		it->writeXML(e);
	}
}


bool Schedule::operator!=(const Schedule & other) const {
	if (m_type != other.m_type) return true;
	if (m_dailyCycles != other.m_dailyCycles) return true;

	return false;
}

int Schedule::priority(type_t scheduleType) {
	switch(scheduleType)
	{
	case ST_ALLDAYS :
		return 10;
		break;
	case ST_WEEKDAY :
		return 8;
		break;
	case ST_WEEKEND :
		return 9;
		break;
	case ST_HOLIDAY :
		return 0;
		break;
	case ST_MONDAY :
		return 1;
		break;
	case ST_TUESDAY :
		return 2;
		break;
	case ST_WEDNESDAY :
		return 3;
		break;
	case ST_THURSDAY :
		return 4;
		break;
	case ST_FRIDAY :
		return 5;
		break;
	case ST_SATURDAY :
		return 6;
		break;
	case ST_SUNDAY :
		return 7;
		break;
	case NUM_ST :
		return -1;
		break;
	}
	return -1;
}


void Schedule::collectQuantities(const std::string & prefix, std::map<std::string, IBK::Unit> & quantities) const {
#if 0
	const char * const FUNC_ID = "[Schedule::collectQuantities]";
	// loop over all dailycycles
	for (unsigned int i=0; i<m_dailyCycles.size(); ++i) {
		// loop over all intervals
		for (unsigned int j=0; j<m_dailyCycles[i].m_intervals.size(); ++j) {
			// loop over all defined paramaters that are not End, Start, or Duration
			std::map<std::string, IBK::Parameter>::const_iterator paraIt =
				m_dailyCycles[i].m_intervals[j].m_genericParaConst.begin();

			for ( ; paraIt != m_dailyCycles[i].m_intervals[j].m_genericParaConst.end(); ++paraIt) {
				std::string name = prefix + ":" + paraIt->second.name;
				IBK::Unit unit = paraIt->second.IO_unit;
				// check if we have this quantity already
				std::map<std::string, IBK::Unit>::iterator it = quantities.find(name);
				if (it != quantities.end()) {
					// check that base unit is the same
					if (it->second.base_id() != unit.base_id()) {
						throw IBK::Exception(IBK::FormatString("Scheduled Quantity '%1' has unit '%2' which mismatches previously "
							"defined quantity with same name but unit '%3' with different base unit '%4'.")
							.arg(name).arg(unit.name()).arg(it->second.name()).arg(unit.base_unit().name()), FUNC_ID);
					}
					// emit warning if IO unit differs
					if (it->second != unit) {
						IBK::IBK_Message( IBK::FormatString("Scheduled Quantity '%1' has unit '%2' which mismatches previously "
							"defined quantity with same name but unit '%3'. Using unit '%2' for outputs.")
							.arg(name).arg(unit.name()).arg(it->second.name()),
							IBK::MSG_WARNING, FUNC_ID, 2);
					}
				}
				else
					quantities[name] = unit;
			}
		}
		// also process intervals
		for (unsigned int j=0; j<m_dailyCycles[i].m_hourlyValues.size(); ++j) {
			std::string name = prefix + ":" + m_dailyCycles[i].m_hourlyValues[j].m_name;
			IBK::Unit unit;
			try {
				unit = IBK::Unit(m_dailyCycles[i].m_hourlyValues[j].m_yUnit);
			}
			catch (...) {
				throw IBK::Exception( IBK::FormatString("Unit '%1' defined for schedule parameter '%2' (hourly values)  is not recognized.")
									  .arg(m_dailyCycles[i].m_hourlyValues[j].m_yUnit.name()).arg(name), FUNC_ID);
			}
			// check if we have this quantity already
			std::map<std::string, IBK::Unit>::iterator it = quantities.find(name);
			if (it != quantities.end()) {
					// check that base unit is the same
					if (it->second.base_id() != unit.base_id()) {
						throw IBK::Exception(IBK::FormatString("Scheduled Quantity '%1' has unit '%2' which mismatches previously "
							"defined quantity with same name but unit '%3' with different base unit '%4'.")
							.arg(name).arg(unit.name()).arg(it->second.name()).arg(unit.base_unit().name()), FUNC_ID);
					}
					// emit warning if IO unit differs
					if (it->second != unit) {
						IBK::IBK_Message( IBK::FormatString("Scheduled Quantity '%1' has unit '%2' which mismatches previously "
							"defined quantity with same name but unit '%3'. Using unit '%2' for outputs.")
							.arg(name).arg(unit.name()).arg(it->second.name()),
							IBK::MSG_WARNING, FUNC_ID, 2);
					}
			}
			else
				quantities[name] = unit;
		}
	}
#endif
}


//void Schedule::collectValidDays(const unsigned int year,
//			const std::list<Date> &defaultHolidays,
//			const std::vector<day_t> &defaultWeekEndDays,
//			std::vector<IBK::Time> &time, bool &allDays) const
//{
//	const char * const FUNC_ID = "[Schedule::collectValidDays]";
//	// usually allDays attribute is not set
//	allDays = false;
//	// parse all schedule definitions
//	switch(m_type)
//	{
//		case NANDRAD::Schedule::ST_HOLIDAY:
//		{
//			std::list<Date> holidays;
//			// do we have a special weekend day definition
//			if(!m_holidays.empty())
//				holidays = m_holidays;
//			else
//				holidays = defaultHolidays;
//			// no holidays definition: error
//			if(holidays.empty())
//					throw IBK::Exception(IBK::FormatString("Error reading holiday definition of schedule of type %1. "
//						"Neither default nor special holidays are defined!")
//						.arg(NANDRAD::KeywordList::Keyword("Schedule::type_t", m_type)),
//						FUNC_ID);

//			// we exactely know the number of days
//			time.resize(holidays.size());

//			std::list<NANDRAD::Date>::const_iterator it = holidays.begin();
//			std::vector<IBK::Time>::iterator timeIt = time.begin();

//			//for(; it != holidays.end(); ++it, ++timeIt)
//			//{
//			//	NANDRAD::Date::convertDateToIBKTime(year, *it, *timeIt);
//			//}
//		}
//		break;
//		case NANDRAD::Schedule::ST_MONDAY:
//			NANDRAD::Schedule::convertWeekDaysToIBKTime(year, std::vector<day_t>(1,SD_MONDAY), time);
//		break;
//		case NANDRAD::Schedule::ST_TUESDAY:
//			NANDRAD::Schedule::convertWeekDaysToIBKTime(year, std::vector<day_t>(1,SD_TUESDAY), time);
//		break;
//		case NANDRAD::Schedule::ST_WEDNESDAY:
//			NANDRAD::Schedule::convertWeekDaysToIBKTime(year, std::vector<day_t>(1,SD_WEDNESDAY), time);
//		break;
//		case NANDRAD::Schedule::ST_THURSDAY:
//			NANDRAD::Schedule::convertWeekDaysToIBKTime(year, std::vector<day_t>(1,SD_THURSDAY), time);
//		break;
//		case NANDRAD::Schedule::ST_FRIDAY:
//			NANDRAD::Schedule::convertWeekDaysToIBKTime(year, std::vector<day_t>(1,SD_FRIDAY), time);
//		break;
//		case NANDRAD::Schedule::ST_SATURDAY:
//			NANDRAD::Schedule::convertWeekDaysToIBKTime(year, std::vector<day_t>(1,SD_SATURDAY), time);
//		break;
//		case NANDRAD::Schedule::ST_SUNDAY:
//			NANDRAD::Schedule::convertWeekDaysToIBKTime(year, std::vector<day_t>(1,SD_SUNDAY), time);
//		break;
//		case NANDRAD::Schedule::ST_WEEKEND:
//		{
//			std::vector<day_t> weekEndDays;
//			// do we have a special weekend day definition
//			if(!m_weekEndDays.empty())
//				weekEndDays = m_weekEndDays;
//			else
//				weekEndDays = defaultWeekEndDays;
//			// no weekend definition: error
//			if(weekEndDays.empty())
//					throw IBK::Exception(IBK::FormatString("Error reading week end definition of schedule of type %1. "
//						"Neither default nor special weekend days are defined!")
//						.arg(NANDRAD::KeywordList::Keyword("Schedule::type_t", m_type)),
//						FUNC_ID);

//			NANDRAD::Schedule::convertWeekDaysToIBKTime(year, weekEndDays, time);
//		}
//		break;
//		case NANDRAD::Schedule::ST_WEEKDAY :
//		{
//			std::vector<day_t> weekEndDays;
//			// do we have a special weekend day definition
//			if(!m_weekEndDays.empty())
//				weekEndDays = m_weekEndDays;
//			else
//				weekEndDays = defaultWeekEndDays;

//			// no weekend definition: error
//			if(weekEndDays.empty())
//					throw IBK::Exception(IBK::FormatString("Error reading week end definition of schedule of type %1. "
//						"Neither default nor special weekend days are defined!")
//						.arg(NANDRAD::KeywordList::Keyword("Schedule::type_t", m_type)),
//						FUNC_ID);

//			// take week definition and cut out all weekend days
//			std::vector<NANDRAD::Schedule::day_t> weekDays(NANDRAD::Schedule::NUM_SD);
//			std::vector<NANDRAD::Schedule::day_t>::iterator it = weekDays.begin();
//			unsigned int w = 0;

//			for(; it !=weekDays.end() ; ++it, ++w)
//				*it = (NANDRAD::Schedule::day_t) w;

//			for(w = 0; w < m_weekEndDays.size(); ++w)
//				weekDays.erase(weekDays.begin() + weekEndDays[w],
//								weekDays.begin() + weekEndDays[w] + 1);

//			NANDRAD::Schedule::convertWeekDaysToIBKTime(year, weekDays, time);
//		}
//		break;
//		case NANDRAD::Schedule::ST_ALLDAYS :
//		{
//			// just set attribute
//			allDays = true;
//		}
//		break;
//		default:
//		break;
//	}
//}



//void Schedule::convertWeekDaysToIBKTime(const unsigned int year, const std::vector<day_t> &weekdays, std::vector<IBK::Time> &time)
//{
//	NANDRAD::Date yearBegin;
//	yearBegin.m_day = 1; yearBegin.m_month = 1;
//	// calculate the difference to the beginning week day
//	NANDRAD::Schedule::day_t firstDayOfYear = (day_t) calcWeekDay(year,yearBegin);
//	std::vector<unsigned int> dayDiff;
//	for(unsigned int i = 0; i < weekdays.size(); ++i)
//	{
//		if(weekdays[i] >= firstDayOfYear)
//			dayDiff.push_back(weekdays[i] - firstDayOfYear);
//		else
//			dayDiff.push_back(weekdays[i] + 7 - firstDayOfYear);
//	}
//	// sort differences
//	std::sort(dayDiff.begin(),dayDiff.end());
//	// now translate into seconds
//	for(unsigned int i = 0; i < 365; i += 7)
//	{
//		for(unsigned int j = 0; j < weekdays.size(); ++j)
//		{
//			// calculate current interval
//			if(dayDiff[j] < 365) {
//				// use time in years and seconds
//				time.push_back(IBK::Time(year, 24 * dayDiff[j] * 3600) );
//			}
//			// update difference to start day for one week
//			dayDiff[j] += 7;
//		}
//	}
//}


//unsigned int Schedule::calcWeekDay(const int year, const NANDRAD::Date &date){
//	const unsigned int monthNumbers[12] = {0,3,3,6,1,4,6,2,5,0,3,5};
//	// day number
//	unsigned int dayNumber = date.m_day % 7;
//	// month number
//	unsigned int monthNumber = monthNumbers[date.m_month - 1];
//	// year number
//	unsigned int yearInCentury = year % 100;
//	unsigned int yearNumber = (yearInCentury + yearInCentury/4) % 7;
//	// century number
//	unsigned int century = (year - yearInCentury) / 100;
//	unsigned int centuryNumber = (3 - century % 4) * 2;
//	// check if a leap year occurs
//	bool leapYear = year % 4 == 0;
//	// calculate a correction
//	unsigned int correction = (leapYear && monthNumber < 3) ? 6 : 0;
//	// now calculate week day: Monday = 0
//	return ( (centuryNumber + yearNumber + monthNumber + dayNumber + correction) % 7 + 6) % 7;
//}


} // namespace NANDRAD

