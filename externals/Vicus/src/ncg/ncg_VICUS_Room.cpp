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

#include <VICUS_Room.h>
#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <VICUS_Constants.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace VICUS {

void Room::readXML(const TiXmlElement * element) {
	FUNCID(Room::readXML);

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
			else if (attribName == "visible")
				m_visible = NANDRAD::readPODAttributeValue<bool>(element, attrib);
			else if (attribName == "ifcGUID")
				m_ifcGUID = attrib->ValueStr();
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
			if (cName == "IdZoneTemplate")
				m_idZoneTemplate = (IDType)NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "IdAcousticTemplate")
				m_idAcousticTemplate = (IDType)NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "AcousticBuildingTypeId")
				m_acousticBuildingTypeId = (IDType)NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "IBK:Parameter") {
				IBK::Parameter p;
				NANDRAD::readParameterElement(c, p);
				bool success = false;
				para_t ptype;
				try {
					ptype = (para_t)KeywordList::Enumeration("Room::para_t", p.name);
					m_para[ptype] = p; success = true;
				}
				catch (...) { /* intentional fail */  }
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.name).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else if (cName == "Surfaces") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Surface")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					Surface obj;
					obj.readXML(c2);
					m_surfaces.push_back(obj);
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
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Room' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Room' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * Room::writeXML(TiXmlElement * parent) const {
	if (m_id == VICUS::INVALID_ID)  return nullptr;
	TiXmlElement * e = new TiXmlElement("Room");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_displayName.isEmpty())
		e->SetAttribute("displayName", m_displayName.toStdString());
	if (m_visible != Room().m_visible)
		e->SetAttribute("visible", IBK::val2string<bool>(m_visible));
	if (!m_ifcGUID.empty())
		e->SetAttribute("ifcGUID", m_ifcGUID);
	if (m_idZoneTemplate != VICUS::INVALID_ID)
			TiXmlElement::appendSingleAttributeElement(e, "IdZoneTemplate", nullptr, std::string(), IBK::val2string<unsigned int>(m_idZoneTemplate));
	if (m_idAcousticTemplate != VICUS::INVALID_ID)
			TiXmlElement::appendSingleAttributeElement(e, "IdAcousticTemplate", nullptr, std::string(), IBK::val2string<unsigned int>(m_idAcousticTemplate));
	if (m_acousticBuildingTypeId != VICUS::INVALID_ID)
			TiXmlElement::appendSingleAttributeElement(e, "AcousticBuildingTypeId", nullptr, std::string(), IBK::val2string<unsigned int>(m_acousticBuildingTypeId));

	for (unsigned int i=0; i<NUM_P; ++i) {
		if (!m_para[i].name.empty()) {
			TiXmlElement::appendIBKParameterElement(e, m_para[i].name, m_para[i].IO_unit.name(), m_para[i].get_value(m_para[i].IO_unit));
		}
	}

	if (!m_surfaces.empty()) {
		TiXmlElement * child = new TiXmlElement("Surfaces");
		e->LinkEndChild(child);

		for (std::vector<Surface>::const_iterator it = m_surfaces.begin();
			it != m_surfaces.end(); ++it)
		{
			it->writeXML(child);
		}
	}

	return e;
}

} // namespace VICUS
