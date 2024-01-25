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

void OutputDefinition::readXML(const TiXmlElement * element) {
	FUNCID(OutputDefinition::readXML);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "quantity"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'quantity' attribute.") ), FUNC_ID);

		if (!TiXmlAttribute::attributeByName(element, "sourceObjectType"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'sourceObjectType' attribute.") ), FUNC_ID);

		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "quantity")
				m_quantity = attrib->ValueStr();
			else if (attribName == "sourceObjectType")
				m_sourceObjectType = attrib->ValueStr();
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}
		// search for mandatory elements
		if (!element->FirstChildElement("SourceObjectIds"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'SourceObjectIds' element.") ), FUNC_ID);

		if (!element->FirstChildElement("GridName"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'GridName' element.") ), FUNC_ID);

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "SourceObjectIds")
				NANDRAD::readVector(c, "SourceObjectIds", m_sourceObjectIds);
			else if (cName == "VectorIdMap")
				m_vectorIdMap.setEncodedString(c->GetText());
			else if (cName == "GridName")
				m_gridName = c->GetText();
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

TiXmlElement * OutputDefinition::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("OutputDefinition");
	parent->LinkEndChild(e);

	if (!m_quantity.empty())
		e->SetAttribute("quantity", m_quantity);
	if (!m_sourceObjectType.empty())
		e->SetAttribute("sourceObjectType", m_sourceObjectType);

	if (m_timeType != NUM_OTT)
		TiXmlElement::appendSingleAttributeElement(e, "TimeType", nullptr, std::string(), KeywordList::Keyword("OutputDefinition::timeType_t",  m_timeType));
	NANDRAD::writeVector(e, "SourceObjectIds", m_sourceObjectIds);
	if (!m_vectorIdMap.m_values.empty())
		TiXmlElement::appendSingleAttributeElement(e, "VectorIdMap", nullptr, std::string(), m_vectorIdMap.encodedString());
	if (!m_gridName.empty())
		TiXmlElement::appendSingleAttributeElement(e, "GridName", nullptr, std::string(), m_gridName);
	return e;
}

} // namespace VICUS
