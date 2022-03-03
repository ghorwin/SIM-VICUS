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

#include <NANDRAD_HydraulicNetworkControlElement.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <NANDRAD_Constants.h>
#include <NANDRAD_KeywordList.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace NANDRAD {

void HydraulicNetworkControlElement::readXML(const TiXmlElement * element) {
	FUNCID(HydraulicNetworkControlElement::readXML);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'id' attribute.") ), FUNC_ID);

		if (!TiXmlAttribute::attributeByName(element, "modelType"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'modelType' attribute.") ), FUNC_ID);

		if (!TiXmlAttribute::attributeByName(element, "controlledProperty"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'controlledProperty' attribute.") ), FUNC_ID);

		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = (IDType)NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "modelType")
				try {
					m_modelType = (ModelType)KeywordList::Enumeration("HydraulicNetworkControlElement::ModelType", attrib->ValueStr());
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
						IBK::FormatString("Invalid or unknown keyword '"+attrib->ValueStr()+"'.") ), FUNC_ID);
				}
			else if (attribName == "controllerType")
				try {
					m_controllerType = (ControllerType)KeywordList::Enumeration("HydraulicNetworkControlElement::ControllerType", attrib->ValueStr());
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
						IBK::FormatString("Invalid or unknown keyword '"+attrib->ValueStr()+"'.") ), FUNC_ID);
				}
			else if (attribName == "controlledProperty")
				try {
					m_controlledProperty = (ControlledProperty)KeywordList::Enumeration("HydraulicNetworkControlElement::ControlledProperty", attrib->ValueStr());
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
						IBK::FormatString("Invalid or unknown keyword '"+attrib->ValueStr()+"'.") ), FUNC_ID);
				}
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
			if (cName == "MaximumControllerResultValue")
				m_maximumControllerResultValue = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "IBK:Parameter") {
				IBK::Parameter p;
				NANDRAD::readParameterElement(c, p);
				bool success = false;
				para_t ptype;
				try {
					ptype = (para_t)KeywordList::Enumeration("HydraulicNetworkControlElement::para_t", p.name);
					m_para[ptype] = p; success = true;
				}
				catch (...) { /* intentional fail */  }
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.name).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else {
				bool found = false;
				for (int i=0; i<NUM_ID; ++i) {
					if (cName == KeywordList::Keyword("HydraulicNetworkControlElement::References",i)) {
						m_idReferences[i] = (IDType)NANDRAD::readPODElement<unsigned int>(c, cName);
						found = true;
						break;
					}
				}
				if (!found)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'HydraulicNetworkControlElement' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'HydraulicNetworkControlElement' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * HydraulicNetworkControlElement::writeXML(TiXmlElement * parent) const {
	if (m_id == NANDRAD::INVALID_ID)  return nullptr;
	TiXmlElement * e = new TiXmlElement("HydraulicNetworkControlElement");
	parent->LinkEndChild(e);

	if (m_id != NANDRAD::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<IDType>(m_id));
	if (m_modelType != NUM_MT)
		e->SetAttribute("modelType", KeywordList::Keyword("HydraulicNetworkControlElement::ModelType",  m_modelType));
	if (m_controllerType != NUM_CT)
		e->SetAttribute("controllerType", KeywordList::Keyword("HydraulicNetworkControlElement::ControllerType",  m_controllerType));
	if (m_controlledProperty != NUM_CP)
		e->SetAttribute("controlledProperty", KeywordList::Keyword("HydraulicNetworkControlElement::ControlledProperty",  m_controlledProperty));

	for (int i=0; i<NUM_ID; ++i) {
		if (m_idReferences[i] != NANDRAD::INVALID_ID)
				TiXmlElement::appendSingleAttributeElement(e, KeywordList::Keyword("HydraulicNetworkControlElement::References",  i), nullptr, std::string(), IBK::val2string<unsigned int>(m_idReferences[i]));
	}
	TiXmlElement::appendSingleAttributeElement(e, "MaximumControllerResultValue", nullptr, std::string(), IBK::val2string<double>(m_maximumControllerResultValue));

	for (unsigned int i=0; i<NUM_P; ++i) {
		if (!m_para[i].name.empty()) {
			TiXmlElement::appendIBKParameterElement(e, m_para[i].name, m_para[i].IO_unit.name(), m_para[i].get_value(m_para[i].IO_unit));
		}
	}
	return e;
}

} // namespace NANDRAD
