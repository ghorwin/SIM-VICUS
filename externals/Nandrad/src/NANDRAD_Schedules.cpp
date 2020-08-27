/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

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

#include "NANDRAD_Schedules.h"
#include "NANDRAD_KeywordList.h"
#include "NANDRAD_Utilities.h"
#include "NANDRAD_Constants.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <IBK_messages.h>

#include <tinyxml.h>

namespace NANDRAD {


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

				std::vector<NANDRAD::LinearSplineParameter> schedules;
				// now read all the schedule subtags

				const TiXmlElement * c3 = c2->FirstChildElement();
				while (c3) {
					const std::string & c3Name = c3->ValueStr();
					if (c3Name != "AnnualSchedule")
						throw IBK::Exception(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c3Name).arg(c3->Row()), FUNC_ID);

					NANDRAD::LinearSplineParameter spl;
					try {
						spl.readXML(c2);
					}
					catch (IBK::Exception & ex) {
						throw IBK::Exception(ex, IBK::FormatString(XML_READ_ERROR).arg(c2->Row())
											 .arg("Invalid data in 'IBK:LinearSpline' tag."), FUNC_ID);
					}
					schedules.push_back(spl);

					c3 = c3->NextSiblingElement();
				}

				m_annualSchedules[objectListName] = schedules;

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
		for (const std::pair<const std::string, std::vector<NANDRAD::LinearSplineParameter> > & svec : m_annualSchedules) {
			// create tags like
			// <AnnualSchedule objectList="objectListName">
			//   ...
			// </AnnualSchedule>
			TiXmlElement * g = new TiXmlElement("AnnualSchedule");
			c->LinkEndChild(g);
			g->SetAttribute("objectList", svec.first);
			for (const NANDRAD::LinearSplineParameter & s : svec.second)
				s.writeXML(g);
		}
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


void Schedules::checkParameters() {
	// we collect all parameter names and their associated object lists
	// then we check, that the same parameter is not defined several times for the same object list

	// Note: there is a problem with duplicate definitions that is not easy to detect:
	//       you may define an annual schedule parameter for "HeatingSetPoint" for zone object list "heated offices"
	//       and also define a daily-cycle based parameter "HeatingSetPoint" for zone object list "ground floor offices".
	//       If now the same zone is referenced by both object lists, there is a redundant definition of
	//       the "HeatingSetPoint" definition, which is not allowed.
	//       We cannot detect this until all object lists are resolved, so we leave this check to the schedule
	//       model when it resolves the value references

}


} // namespace NANDRAD

