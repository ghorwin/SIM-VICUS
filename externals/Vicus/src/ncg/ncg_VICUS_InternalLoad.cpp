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

#include <VICUS_InternalLoad.h>
#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <VICUS_Constants.h>
#include <NANDRAD_Utilities.h>
#include <VICUS_KeywordList.h>

#include <tinyxml.h>

namespace VICUS {

void InternalLoad::readXML(const TiXmlElement * element) {
	FUNCID(InternalLoad::readXML);

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
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}
		// search for mandatory elements
		if (!element->FirstChildElement("Category"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'Category' element.") ), FUNC_ID);

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "IdOccupancySchedule")
				m_idOccupancySchedule = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "IdActivitySchedule")
				m_idActivitySchedule = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "IdMoistureProductionRatePerAreaSchedule")
				m_idMoistureProductionRatePerAreaSchedule = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "IdPowerManagementSchedule")
				m_idPowerManagementSchedule = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "ActivateCO2Production")
				m_activateCO2Production = NANDRAD::readPODElement<bool>(c, cName);
			else if (cName == "IBK:Parameter") {
				IBK::Parameter p;
				NANDRAD::readParameterElement(c, p);
				bool success = false;
				para_t ptype;
				try {
					ptype = (para_t)KeywordList::Enumeration("InternalLoad::para_t", p.name);
					m_para[ptype] = p; success = true;
				}
				catch (...) { /* intentional fail */  }
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.name).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else if (cName == "Category") {
				try {
					m_category = (Category)KeywordList::Enumeration("InternalLoad::Category", c->GetText());
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
						IBK::FormatString("Invalid or unknown keyword '"+std::string(c->GetText())+"'.") ), FUNC_ID);
				}
			}
			else if (cName == "PersonCountMethod") {
				try {
					m_personCountMethod = (PersonCountMethod)KeywordList::Enumeration("InternalLoad::PersonCountMethod", c->GetText());
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
						IBK::FormatString("Invalid or unknown keyword '"+std::string(c->GetText())+"'.") ), FUNC_ID);
				}
			}
			else if (cName == "PowerMethod") {
				try {
					m_powerMethod = (PowerMethod)KeywordList::Enumeration("InternalLoad::PowerMethod", c->GetText());
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
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'InternalLoad' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'InternalLoad' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * InternalLoad::writeXML(TiXmlElement * parent) const {
	if (m_id == VICUS::INVALID_ID)  return nullptr;
	TiXmlElement * e = new TiXmlElement("InternalLoad");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_displayName.empty())
		e->SetAttribute("displayName", m_displayName.encodedString());
	if (m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());

	if (m_category != NUM_MC)
		TiXmlElement::appendSingleAttributeElement(e, "Category", nullptr, std::string(), KeywordList::Keyword("InternalLoad::Category",  m_category));

	if (m_personCountMethod != NUM_PCM)
		TiXmlElement::appendSingleAttributeElement(e, "PersonCountMethod", nullptr, std::string(), KeywordList::Keyword("InternalLoad::PersonCountMethod",  m_personCountMethod));

	if (m_powerMethod != NUM_PM)
		TiXmlElement::appendSingleAttributeElement(e, "PowerMethod", nullptr, std::string(), KeywordList::Keyword("InternalLoad::PowerMethod",  m_powerMethod));
	if (m_idOccupancySchedule != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdOccupancySchedule", nullptr, std::string(), IBK::val2string<unsigned int>(m_idOccupancySchedule));
	if (m_idActivitySchedule != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdActivitySchedule", nullptr, std::string(), IBK::val2string<unsigned int>(m_idActivitySchedule));
	if (m_idMoistureProductionRatePerAreaSchedule != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdMoistureProductionRatePerAreaSchedule", nullptr, std::string(), IBK::val2string<unsigned int>(m_idMoistureProductionRatePerAreaSchedule));
	if (m_idPowerManagementSchedule != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdPowerManagementSchedule", nullptr, std::string(), IBK::val2string<unsigned int>(m_idPowerManagementSchedule));
	TiXmlElement::appendSingleAttributeElement(e, "ActivateCO2Production", nullptr, std::string(), IBK::val2string<bool>(m_activateCO2Production));

	for (unsigned int i=0; i<NUM_P; ++i) {
		if (!m_para[i].name.empty()) {
			TiXmlElement::appendIBKParameterElement(e, m_para[i].name, m_para[i].IO_unit.name(), m_para[i].get_value(m_para[i].IO_unit));
		}
	}
	return e;
}

} // namespace VICUS
