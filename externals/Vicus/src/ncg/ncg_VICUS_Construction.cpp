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

#include <VICUS_Construction.h>
#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <VICUS_Constants.h>
#include <NANDRAD_Utilities.h>
#include <VICUS_Constants.h>

#include <tinyxml.h>

namespace VICUS {

void Construction::readXML(const TiXmlElement * element) {
	FUNCID(Construction::readXML);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'id' attribute.") ), FUNC_ID);

		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "displayName")
				m_displayName = QString::fromStdString(attrib->ValueStr());
			else if (attribName == "color")
				m_color.setNamedColor(QString::fromStdString(attrib->ValueStr()));
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}
		// search for mandatory elements
		if (!element->FirstChildElement("IsOpaque"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'IsOpaque' element.") ), FUNC_ID);

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "Notes")
				m_notes = QString::fromStdString(c->GetText());
			else if (cName == "Manufacturer")
				m_manufacturer = QString::fromStdString(c->GetText());
			else if (cName == "DataSource")
				m_dataSource = QString::fromStdString(c->GetText());
			else if (cName == "IBK:Flag") {
				IBK::Flag f;
				NANDRAD::readFlagElement(c, f);
				bool success = false;
				if (f.name() == "IsOpaque") {
					m_isOpaque = f; success=true;
				}
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(f.name()).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else if (cName == "MaterialLayers") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "MaterialLayer")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					MaterialLayer obj;
					obj.readXML(c2);
					m_materialLayers.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "IdFrame")
				m_idFrame = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "IdDivider")
				m_idDivider = NANDRAD::readPODElement<unsigned int>(c, cName);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Construction' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Construction' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * Construction::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("Construction");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_displayName.isEmpty())
		e->SetAttribute("displayName", m_displayName.toStdString());
	if (!m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());
	if (!m_notes.isEmpty())
		TiXmlElement::appendSingleAttributeElement(e, "Notes", nullptr, std::string(), m_notes.toStdString());
	if (!m_manufacturer.isEmpty())
		TiXmlElement::appendSingleAttributeElement(e, "Manufacturer", nullptr, std::string(), m_manufacturer.toStdString());
	if (!m_dataSource.isEmpty())
		TiXmlElement::appendSingleAttributeElement(e, "DataSource", nullptr, std::string(), m_dataSource.toStdString());
	if (!m_isOpaque.name().empty()) {
		IBK_ASSERT("IsOpaque" == m_isOpaque.name());
		TiXmlElement::appendSingleAttributeElement(e, "IBK:Flag", "name", "IsOpaque", m_isOpaque.isEnabled() ? "true" : "false");
	}

	if (!m_materialLayers.empty()) {
		TiXmlElement * child = new TiXmlElement("MaterialLayers");
		e->LinkEndChild(child);

		for (std::vector<MaterialLayer>::const_iterator it = m_materialLayers.begin();
			it != m_materialLayers.end(); ++it)
		{
			it->writeXML(child);
		}
	}

	if (m_idFrame != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdFrame", nullptr, std::string(), IBK::val2string<unsigned int>(m_idFrame));
	if (m_idDivider != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdDivider", nullptr, std::string(), IBK::val2string<unsigned int>(m_idDivider));
	return e;
}

} // namespace VICUS
