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

#include <NANDRAD_ControlElement.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <NANDRAD_Constants.h>
#include <NANDRAD_KeywordList.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace NANDRAD {

void ControlElement::readXML(const TiXmlElement * element) {
	FUNCID(ControlElement::readXML);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "controlType"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'controlType' attribute.") ), FUNC_ID);

		if (!TiXmlAttribute::attributeByName(element, "controllerId"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'controllerId' attribute.") ), FUNC_ID);

		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "controlType")
			try {
				m_controlType = (ControlType)KeywordList::Enumeration("ControlElement::ControlType", attrib->ValueStr());
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
					IBK::FormatString("Invalid or unknown keyword '"+attrib->ValueStr()+"'.") ), FUNC_ID);
			}
			if (attribName == "controllerId")
				m_controllerId = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
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
				else if (p.name == "MaximumControllerError") {
					m_maximumControllerError = p; success = true;
				}
				else if (p.name == "MaximumSystemInput") {
					m_maximumSystemInput = p; success = true;
				}
				if (!success) {
				}
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.name).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else if (cName == "LinearSplineParameter") {
				NANDRAD::LinearSplineParameter p;
				p.readXML(c);
				bool success = false;
				if (p.m_name == "SetPointSpline") {
					m_setPointSpline = p; success = true;
				}
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.m_name).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else if (cName == "SetPointScheduleName")
				m_setPointScheduleName = c->GetText();
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'ControlElement' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'ControlElement' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * ControlElement::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("ControlElement");
	parent->LinkEndChild(e);

	if (m_controlType != NUM_CT)
		e->SetAttribute("controlType", KeywordList::Keyword("ControlElement::ControlType",  m_controlType));
	if (m_controllerId != NANDRAD::INVALID_ID)
		e->SetAttribute("controllerId", IBK::val2string<unsigned int>(m_controllerId));
	if (!m_setPoint.name.empty()) {
		IBK_ASSERT("SetPoint" == m_setPoint.name);
		TiXmlElement::appendIBKParameterElement(e, "SetPoint", m_setPoint.IO_unit.name(), m_setPoint.get_value());
	}
	if (!m_setPointSpline.m_name.empty()) {
		IBK_ASSERT("SetPointSpline" == m_setPointSpline.m_name);
		m_setPointSpline.writeXML(e);
	}
	if (!m_setPointScheduleName.empty())
		TiXmlElement::appendSingleAttributeElement(e, "SetPointScheduleName", nullptr, std::string(), m_setPointScheduleName);
	if (!m_maximumControllerError.name.empty()) {
		IBK_ASSERT("MaximumControllerError" == m_maximumControllerError.name);
		TiXmlElement::appendIBKParameterElement(e, "MaximumControllerError", m_maximumControllerError.IO_unit.name(), m_maximumControllerError.get_value());
	}
	if (!m_maximumSystemInput.name.empty()) {
		IBK_ASSERT("MaximumSystemInput" == m_maximumSystemInput.name);
		TiXmlElement::appendIBKParameterElement(e, "MaximumSystemInput", m_maximumSystemInput.IO_unit.name(), m_maximumSystemInput.get_value());
	}
	return e;
}

} // namespace NANDRAD
