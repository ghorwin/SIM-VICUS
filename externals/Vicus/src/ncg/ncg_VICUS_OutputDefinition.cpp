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

#include <VICUS_OutputDefinition.h>
#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <VICUS_Constants.h>
#include <NANDRAD_Utilities.h>
#include <VICUS_KeywordList.h>

#include <tinyxml.h>

namespace VICUS {

void OutputDefinition::readXMLPrivate(const TiXmlElement * element) {
	FUNCID(OutputDefinition::readXMLPrivate);

	try {
		// search for mandatory attributes
		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "name")
				m_name = attrib->ValueStr();
			else if (attribName == "type")
				m_type = attrib->ValueStr();
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
			if (cName == "Unit")
				m_unit = NANDRAD::readUnitElement(c, cName);
			else if (cName == "VectorIds")
				NANDRAD::readVector(c, "VectorIds", m_vectorIds);
			else if (cName == "SourceObjectIds")
				NANDRAD::readVector(c, "SourceObjectIds", m_sourceObjectIds);
			else if (cName == "ActiveSourceObjectIds")
				NANDRAD::readVector(c, "ActiveSourceObjectIds", m_activeSourceObjectIds);
			else if (cName == "TimeType") {
				try {
					m_timeType = (timeType_t)KeywordList::Enumeration("OutputDefinition::timeType_t", c->GetText());
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
						IBK::FormatString("Invalid or unknown keyword '"+std::string(c->GetText())+"'.") ), FUNC_ID);
				}
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'OutputDefinition' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'OutputDefinition' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * OutputDefinition::writeXMLPrivate(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("OutputDefinition");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_name.empty())
		e->SetAttribute("name", m_name);
	if (!m_type.empty())
		e->SetAttribute("type", m_type);
	if (m_unit.id() != 0)
		TiXmlElement::appendSingleAttributeElement(e, "Unit", nullptr, std::string(), m_unit.name());

	if (m_timeType != NUM_OTT)
		TiXmlElement::appendSingleAttributeElement(e, "TimeType", nullptr, std::string(), KeywordList::Keyword("OutputDefinition::timeType_t",  m_timeType));
	NANDRAD::writeVector(e, "VectorIds", m_vectorIds);
	NANDRAD::writeVector(e, "SourceObjectIds", m_sourceObjectIds);
	NANDRAD::writeVector(e, "ActiveSourceObjectIds", m_activeSourceObjectIds);
	return e;
}

} // namespace VICUS
