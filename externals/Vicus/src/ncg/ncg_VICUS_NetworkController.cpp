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

#include <VICUS_NetworkController.h>
#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <VICUS_Constants.h>
#include <NANDRAD_Utilities.h>
#include <VICUS_KeywordList.h>

#include <tinyxml.h>

namespace VICUS {

void NetworkController::readXML(const TiXmlElement * element) {
	FUNCID(NetworkController::readXML);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'id' attribute.") ), FUNC_ID);

		if (!TiXmlAttribute::attributeByName(element, "controllerType"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'controllerType' attribute.") ), FUNC_ID);

		if (!TiXmlAttribute::attributeByName(element, "controlledProperty"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'controlledProperty' attribute.") ), FUNC_ID);

		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "controllerType")
				try {
					m_controllerType = (ControllerType)KeywordList::Enumeration("NetworkController::ControllerType", attrib->ValueStr());
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
						IBK::FormatString("Invalid or unknown keyword '"+attrib->ValueStr()+"'.") ), FUNC_ID);
				}
			else if (attribName == "controlledProperty")
				try {
					m_controlledProperty = (ControlledProperty)KeywordList::Enumeration("NetworkController::ControlledProperty", attrib->ValueStr());
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
						IBK::FormatString("Invalid or unknown keyword '"+attrib->ValueStr()+"'.") ), FUNC_ID);
				}
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
			if (cName == "IBK:Parameter") {
				IBK::Parameter p;
				NANDRAD::readParameterElement(c, p);
				bool success = false;
				if (p.name == "SetPoint") {
					m_setPoint = p; success = true;
				}
				if (!success) {
				para_t ptype;
				try {
					ptype = (para_t)KeywordList::Enumeration("NetworkController::para_t", p.name);
					m_para[ptype] = p; success = true;
				}
				catch (...) { /* intentional fail */  }
				}
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.name).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else if (cName == "MaximumControllerResultValue")
				m_maximumControllerResultValue = NANDRAD::readPODElement<double>(c, cName);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'NetworkController' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'NetworkController' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * NetworkController::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("NetworkController");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (m_controllerType != NUM_CT)
		e->SetAttribute("controllerType", KeywordList::Keyword("NetworkController::ControllerType",  m_controllerType));
	if (m_controlledProperty != NUM_CP)
		e->SetAttribute("controlledProperty", KeywordList::Keyword("NetworkController::ControlledProperty",  m_controlledProperty));
	if (!m_displayName.empty())
		e->SetAttribute("displayName", m_displayName.encodedString());
	if (!m_setPoint.name.empty()) {
		IBK_ASSERT("SetPoint" == m_setPoint.name);
		TiXmlElement::appendIBKParameterElement(e, "SetPoint", m_setPoint.IO_unit.name(), m_setPoint.get_value());
	}
	TiXmlElement::appendSingleAttributeElement(e, "MaximumControllerResultValue", nullptr, std::string(), IBK::val2string<double>(m_maximumControllerResultValue));

	for (unsigned int i=0; i<NUM_P; ++i) {
		if (!m_para[i].name.empty()) {
			TiXmlElement::appendIBKParameterElement(e, m_para[i].name, m_para[i].IO_unit.name(), m_para[i].get_value());
		}
	}
	return e;
}

} // namespace VICUS
