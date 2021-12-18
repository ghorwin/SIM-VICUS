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

#include <VICUS_Outputs.h>
#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <VICUS_Constants.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace VICUS {

void Outputs::readXMLPrivate(const TiXmlElement * element) {
	FUNCID(Outputs::readXMLPrivate);

	try {
		// search for mandatory attributes
		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "checkSum")
				m_checkSum = attrib->ValueStr();
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
			if (cName == "Definitions") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "OutputDefinition")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					VICUS::OutputDefinition obj;
					obj.readXML(c2);
					m_definitions.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Grids") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "OutputGrid")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					NANDRAD::OutputGrid obj;
					obj.readXML(c2);
					m_grids.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "TimeUnit")
				m_timeUnit = NANDRAD::readUnitElement(c, cName);
			else if (cName == "IBK:Flag") {
				IBK::Flag f;
				NANDRAD::readFlagElement(c, f);
				bool success = false;
				try {
					flag_t ftype = (flag_t)KeywordList::Enumeration("Outputs::flag_t", f.name());
					m_flags[ftype] = f; success=true;
				}
				catch (...) { /* intentional fail */  }
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(f.name()).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Outputs' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Outputs' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * Outputs::writeXMLPrivate(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("Outputs");
	parent->LinkEndChild(e);

	if (!m_checkSum.empty())
		e->SetAttribute("checkSum", m_checkSum);

	if (!m_definitions.empty()) {
		TiXmlElement * child = new TiXmlElement("Definitions");
		e->LinkEndChild(child);

		for (std::vector<VICUS::OutputDefinition>::const_iterator it = m_definitions.begin();
			it != m_definitions.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_grids.empty()) {
		TiXmlElement * child = new TiXmlElement("Grids");
		e->LinkEndChild(child);

		for (std::vector<NANDRAD::OutputGrid>::const_iterator it = m_grids.begin();
			it != m_grids.end(); ++it)
		{
			it->writeXML(child);
		}
	}

	if (m_timeUnit.id() != 0)
		TiXmlElement::appendSingleAttributeElement(e, "TimeUnit", nullptr, std::string(), m_timeUnit.name());

	for (int i=0; i<NUM_F; ++i) {
		if (!m_flags[i].name().empty()) {
			TiXmlElement::appendSingleAttributeElement(e, "IBK:Flag", "name", m_flags[i].name(), m_flags[i].isEnabled() ? "true" : "false");
		}
	}
	return e;
}

} // namespace VICUS
