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

#include <VICUS_Component.h>
#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <VICUS_Constants.h>
#include <NANDRAD_Utilities.h>
#include <VICUS_KeywordList.h>

#include <tinyxml.h>

namespace VICUS {

void Component::readXML(const TiXmlElement * element) {
	FUNCID(Component::readXML);

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
		if (!element->FirstChildElement("Type"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'Type' element.") ), FUNC_ID);

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "Notes")
				m_notes.setEncodedString(c->GetText());
			else if (cName == "Manufacturer")
				m_manufacturer.setEncodedString(c->GetText());
			else if (cName == "DataSource")
				m_dataSource.setEncodedString(c->GetText());
			else if (cName == "IdConstruction")
				m_idConstruction = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "IdSideABoundaryCondition")
				m_idSideABoundaryCondition = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "IdSideBBoundaryCondition")
				m_idSideBBoundaryCondition = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "IdSideAAcousticBoundaryCondition")
				m_idSideAAcousticBoundaryCondition = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "IdSideBAcousticBoundaryCondition")
				m_idSideBAcousticBoundaryCondition = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "IdSurfaceProperty")
				m_idSurfaceProperty = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "ActiveLayerIndex")
				m_activeLayerIndex = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "Type") {
				try {
					m_type = (ComponentType)KeywordList::Enumeration("Component::ComponentType", c->GetText());
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
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Component' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Component' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * Component::writeXML(TiXmlElement * parent) const {
	if (m_id == VICUS::INVALID_ID)  return nullptr;
	TiXmlElement * e = new TiXmlElement("Component");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_displayName.empty())
		e->SetAttribute("displayName", m_displayName.encodedString());
	if (m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());
	if (!m_notes.empty())
		TiXmlElement::appendSingleAttributeElement(e, "Notes", nullptr, std::string(), m_notes.encodedString());
	if (!m_manufacturer.empty())
		TiXmlElement::appendSingleAttributeElement(e, "Manufacturer", nullptr, std::string(), m_manufacturer.encodedString());
	if (!m_dataSource.empty())
		TiXmlElement::appendSingleAttributeElement(e, "DataSource", nullptr, std::string(), m_dataSource.encodedString());

	if (m_type != NUM_CT)
		TiXmlElement::appendSingleAttributeElement(e, "Type", nullptr, std::string(), KeywordList::Keyword("Component::ComponentType",  m_type));
	if (m_idConstruction != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdConstruction", nullptr, std::string(), IBK::val2string<unsigned int>(m_idConstruction));
	if (m_idSideABoundaryCondition != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdSideABoundaryCondition", nullptr, std::string(), IBK::val2string<unsigned int>(m_idSideABoundaryCondition));
	if (m_idSideBBoundaryCondition != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdSideBBoundaryCondition", nullptr, std::string(), IBK::val2string<unsigned int>(m_idSideBBoundaryCondition));
	if (m_idSideAAcousticBoundaryCondition != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdSideAAcousticBoundaryCondition", nullptr, std::string(), IBK::val2string<unsigned int>(m_idSideAAcousticBoundaryCondition));
	if (m_idSideBAcousticBoundaryCondition != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdSideBAcousticBoundaryCondition", nullptr, std::string(), IBK::val2string<unsigned int>(m_idSideBAcousticBoundaryCondition));
	if (m_idSurfaceProperty != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdSurfaceProperty", nullptr, std::string(), IBK::val2string<unsigned int>(m_idSurfaceProperty));
	if (m_activeLayerIndex != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "ActiveLayerIndex", nullptr, std::string(), IBK::val2string<unsigned int>(m_activeLayerIndex));
	return e;
}

} // namespace VICUS
