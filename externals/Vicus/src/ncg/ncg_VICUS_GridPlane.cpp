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

#include <VICUS_GridPlane.h>
#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <VICUS_Constants.h>
#include <IBKMK_Vector3D.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace VICUS {

void GridPlane::readXMLPrivate(const TiXmlElement * element) {
	FUNCID(GridPlane::readXMLPrivate);

	try {
		// search for mandatory attributes
		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "name")
				m_name = QString::fromStdString(attrib->ValueStr());
			else if (attribName == "isVisible")
				m_isVisible = NANDRAD::readPODAttributeValue<bool>(element, attrib);
			else if (attribName == "isActive")
				m_isActive = NANDRAD::readPODAttributeValue<bool>(element, attrib);
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
			if (cName == "Offset") {
				try {
					m_offset = IBKMK::Vector3D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
										  .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else if (cName == "Normal") {
				try {
					m_normal = IBKMK::Vector3D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
										  .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else if (cName == "LocalX") {
				try {
					m_localX = IBKMK::Vector3D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
										  .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else if (cName == "Color")
				m_color.setNamedColor(QString::fromStdString(c->GetText()));
			else if (cName == "Width")
				m_width = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "Spacing")
				m_spacing = NANDRAD::readPODElement<unsigned int>(c, cName);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'GridPlane' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'GridPlane' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * GridPlane::writeXMLPrivate(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("GridPlane");
	parent->LinkEndChild(e);

	if (!m_name.isEmpty())
		e->SetAttribute("name", m_name.toStdString());
	if (m_isVisible != GridPlane().m_isVisible)
		e->SetAttribute("isVisible", IBK::val2string<bool>(m_isVisible));
	if (m_isActive != GridPlane().m_isActive)
		e->SetAttribute("isActive", IBK::val2string<bool>(m_isActive));
	TiXmlElement::appendSingleAttributeElement(e, "Offset", nullptr, std::string(), m_offset.toString());
	TiXmlElement::appendSingleAttributeElement(e, "Normal", nullptr, std::string(), m_normal.toString());
	TiXmlElement::appendSingleAttributeElement(e, "LocalX", nullptr, std::string(), m_localX.toString());
	if (m_color.isValid())
		TiXmlElement::appendSingleAttributeElement(e, "Color", nullptr, std::string(), m_color.name().toStdString());
	if (m_width != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "Width", nullptr, std::string(), IBK::val2string<unsigned int>(m_width));
	if (m_spacing != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "Spacing", nullptr, std::string(), IBK::val2string<unsigned int>(m_spacing));
	return e;
}

} // namespace VICUS
