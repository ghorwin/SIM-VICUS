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

#include <VICUS_EpdDataset.h>
#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <VICUS_Constants.h>
#include <NANDRAD_Utilities.h>
#include <VICUS_KeywordList.h>

#include <tinyxml.h>

namespace VICUS {

void EpdDataset::readXML(const TiXmlElement * element) {
	FUNCID(EpdDataset::readXML);

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
				m_displayName.setEncodedString(attrib->ValueStr());
			else if (attribName == "color")
				m_color.setNamedColor(QString::fromStdString(attrib->ValueStr()));
			else if (attribName == "uuid")
				m_uuid = QString::fromStdString(attrib->ValueStr());
			else if (attribName == "category")
				m_category.setEncodedString(attrib->ValueStr());
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}
		// search for mandatory elements
		if (!element->FirstChildElement("Type"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'Type' element.") ), FUNC_ID);

		if (!element->FirstChildElement("Modules"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'Modules' element.") ), FUNC_ID);

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
			else if (cName == "ExpireYear")
				m_expireYear = QString::fromStdString(c->GetText());
			else if (cName == "ReferenceUnit")
				m_referenceUnit = NANDRAD::readUnitElement(c, cName);
			else if (cName == "ReferenceQuantity")
				m_referenceQuantity = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "Modules")
				m_modules = QString::fromStdString(c->GetText());
			else if (cName == "EpdModuleDataset") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "EpdModuleDataset")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					EpdModuleDataset obj;
					obj.readXML(c2);
					m_epdModuleDataset.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Type") {
				try {
					m_type = (Type)KeywordList::Enumeration("EpdDataset::Type", c->GetText());
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
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'EpdDataset' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'EpdDataset' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * EpdDataset::writeXML(TiXmlElement * parent) const {
	if (m_id == VICUS::INVALID_ID)  return nullptr;
	TiXmlElement * e = new TiXmlElement("EpdDataset");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_displayName.empty())
		e->SetAttribute("displayName", m_displayName.encodedString());
	if (m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());
	if (!m_uuid.isEmpty())
		e->SetAttribute("uuid", m_uuid.toStdString());
	if (!m_category.empty())
		e->SetAttribute("category", m_category.encodedString());
	if (!m_notes.isEmpty())
		TiXmlElement::appendSingleAttributeElement(e, "Notes", nullptr, std::string(), m_notes.toStdString());
	if (!m_manufacturer.isEmpty())
		TiXmlElement::appendSingleAttributeElement(e, "Manufacturer", nullptr, std::string(), m_manufacturer.toStdString());
	if (!m_dataSource.isEmpty())
		TiXmlElement::appendSingleAttributeElement(e, "DataSource", nullptr, std::string(), m_dataSource.toStdString());
	if (!m_expireYear.isEmpty())
		TiXmlElement::appendSingleAttributeElement(e, "ExpireYear", nullptr, std::string(), m_expireYear.toStdString());
	if (m_referenceUnit.id() != 0)
		TiXmlElement::appendSingleAttributeElement(e, "ReferenceUnit", nullptr, std::string(), m_referenceUnit.name());
	TiXmlElement::appendSingleAttributeElement(e, "ReferenceQuantity", nullptr, std::string(), IBK::val2string<double>(m_referenceQuantity));

	if (m_type != NUM_T)
		TiXmlElement::appendSingleAttributeElement(e, "Type", nullptr, std::string(), KeywordList::Keyword("EpdDataset::Type",  m_type));
	if (!m_modules.isEmpty())
		TiXmlElement::appendSingleAttributeElement(e, "Modules", nullptr, std::string(), m_modules.toStdString());

	if (!m_epdModuleDataset.empty()) {
		TiXmlElement * child = new TiXmlElement("EpdModuleDataset");
		e->LinkEndChild(child);

		for (std::vector<EpdModuleDataset>::const_iterator it = m_epdModuleDataset.begin();
			it != m_epdModuleDataset.end(); ++it)
		{
			it->writeXML(child);
		}
	}

	return e;
}

} // namespace VICUS
