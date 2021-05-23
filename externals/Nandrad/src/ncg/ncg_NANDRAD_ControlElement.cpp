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
			else if (attribName == "controllerId")
				m_controllerId = (IDType)NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
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
	e->SetAttribute("controllerId", IBK::val2string<IDType>(m_controllerId));
	if (!m_setPoint.name.empty()) {
		IBK_ASSERT("SetPoint" == m_setPoint.name);
		TiXmlElement::appendIBKParameterElement(e, "SetPoint", m_setPoint.IO_unit.name(), m_setPoint.get_value());
	}
	TiXmlElement::appendSingleAttributeElement(e, "MaximumControllerResultValue", nullptr, std::string(), IBK::val2string<double>(m_maximumControllerResultValue));
	return e;
}

} // namespace NANDRAD
