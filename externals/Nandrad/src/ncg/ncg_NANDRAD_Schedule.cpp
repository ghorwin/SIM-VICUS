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

#include <NANDRAD_Schedule.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <NANDRAD_Constants.h>
#include <NANDRAD_KeywordList.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace NANDRAD {

void Schedule::readXML(const TiXmlElement * element) {
	FUNCID(Schedule::readXML);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "type"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'type' attribute.") ), FUNC_ID);

		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "type")
				try {
					m_type = (ScheduledDayType)KeywordList::Enumeration("Schedule::ScheduledDayType", attrib->ValueStr());
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
						IBK::FormatString("Invalid or unknown keyword '"+attrib->ValueStr()+"'.") ), FUNC_ID);
				}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}
		// search for mandatory elements
		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "StartDayOfTheYear")
				m_startDayOfTheYear = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "EndDayOfTheYear")
				m_endDayOfTheYear = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "DailyCycles") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "DailyCycle")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					DailyCycle obj;
					obj.readXML(c2);
					m_dailyCycles.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Schedule' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Schedule' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * Schedule::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("Schedule");
	parent->LinkEndChild(e);

	if (m_type != NUM_ST)
		e->SetAttribute("type", KeywordList::Keyword("Schedule::ScheduledDayType",  m_type));
	if (m_startDayOfTheYear != NANDRAD::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "StartDayOfTheYear", nullptr, std::string(), IBK::val2string<unsigned int>(m_startDayOfTheYear));
	if (m_endDayOfTheYear != NANDRAD::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "EndDayOfTheYear", nullptr, std::string(), IBK::val2string<unsigned int>(m_endDayOfTheYear));

	if (!m_dailyCycles.empty()) {
		TiXmlElement * child = new TiXmlElement("DailyCycles");
		e->LinkEndChild(child);

		for (std::vector<DailyCycle>::const_iterator it = m_dailyCycles.begin();
			it != m_dailyCycles.end(); ++it)
		{
			it->writeXML(child);
		}
	}

	return e;
}

} // namespace NANDRAD
