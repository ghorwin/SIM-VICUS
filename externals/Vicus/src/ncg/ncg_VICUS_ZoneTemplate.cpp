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

#include <VICUS_ZoneTemplate.h>
#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <VICUS_Constants.h>
#include <NANDRAD_Utilities.h>
#include <VICUS_Constants.h>

#include <tinyxml.h>

namespace VICUS {

void ZoneTemplate::readXML(const TiXmlElement * element) {
	FUNCID(ZoneTemplate::readXML);

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
			if (cName == "Notes")
				m_notes.setEncodedString(c->GetText());
			else if (cName == "DataSource")
				m_dataSource.setEncodedString(c->GetText());
			else if (cName == "IdIntLoadPerson")
				m_idIntLoadPerson = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "IdIntLoadElectricEquipment")
				m_idIntLoadElectricEquipment = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "IdIntLoadLighting")
				m_idIntLoadLighting = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "IdIntLoadOther")
				m_idIntLoadOther = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "IdControlThermostat")
				m_idControlThermostat = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "IdControlShading")
				m_idControlShading = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "IdNaturalVentilation")
				m_idNaturalVentilation = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "IdMechanicalVentilation")
				m_idMechanicalVentilation = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "IdInfiltration")
				m_idInfiltration = NANDRAD::readPODElement<unsigned int>(c, cName);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'ZoneTemplate' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'ZoneTemplate' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * ZoneTemplate::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("ZoneTemplate");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_displayName.empty())
		e->SetAttribute("displayName", m_displayName.encodedString());
	if (!m_notes.empty())
		TiXmlElement::appendSingleAttributeElement(e, "Notes", nullptr, std::string(), m_notes.encodedString());
	if (!m_dataSource.empty())
		TiXmlElement::appendSingleAttributeElement(e, "DataSource", nullptr, std::string(), m_dataSource.encodedString());
	if (m_idIntLoadPerson != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdIntLoadPerson", nullptr, std::string(), IBK::val2string<unsigned int>(m_idIntLoadPerson));
	if (m_idIntLoadElectricEquipment != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdIntLoadElectricEquipment", nullptr, std::string(), IBK::val2string<unsigned int>(m_idIntLoadElectricEquipment));
	if (m_idIntLoadLighting != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdIntLoadLighting", nullptr, std::string(), IBK::val2string<unsigned int>(m_idIntLoadLighting));
	if (m_idIntLoadOther != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdIntLoadOther", nullptr, std::string(), IBK::val2string<unsigned int>(m_idIntLoadOther));
	if (m_idControlThermostat != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdControlThermostat", nullptr, std::string(), IBK::val2string<unsigned int>(m_idControlThermostat));
	if (m_idControlShading != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdControlShading", nullptr, std::string(), IBK::val2string<unsigned int>(m_idControlShading));
	if (m_idNaturalVentilation != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdNaturalVentilation", nullptr, std::string(), IBK::val2string<unsigned int>(m_idNaturalVentilation));
	if (m_idMechanicalVentilation != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdMechanicalVentilation", nullptr, std::string(), IBK::val2string<unsigned int>(m_idMechanicalVentilation));
	if (m_idInfiltration != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdInfiltration", nullptr, std::string(), IBK::val2string<unsigned int>(m_idInfiltration));
	return e;
}

} // namespace VICUS
