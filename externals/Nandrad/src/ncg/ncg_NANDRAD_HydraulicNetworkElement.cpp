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

#include <NANDRAD_HydraulicNetworkElement.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <NANDRAD_Constants.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace NANDRAD {

void HydraulicNetworkElement::readXML(const TiXmlElement * element) {
	FUNCID(HydraulicNetworkElement::readXML);

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
				m_id = (IDType)NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "inletNodeId")
				m_inletNodeId = (IDType)NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "outletNodeId")
				m_outletNodeId = (IDType)NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "inletZoneId")
				m_inletZoneId = (IDType)NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "outletZoneId")
				m_outletZoneId = (IDType)NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "componentId")
				m_componentId = (IDType)NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "pipePropertiesId")
				m_pipePropertiesId = (IDType)NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "controlElementId")
				m_controlElementId = (IDType)NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "displayName")
				m_displayName = attrib->ValueStr();
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
				para_t ptype;
				try {
					ptype = (para_t)KeywordList::Enumeration("HydraulicNetworkElement::para_t", p.name);
					m_para[ptype] = p; success = true;
				}
				catch (...) { /* intentional fail */  }
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.name).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else if (cName == "IBK:IntPara") {
				IBK::IntPara p;
				NANDRAD::readIntParaElement(c, p);
				bool success = false;
				try {
					intPara_t ptype = (intPara_t)KeywordList::Enumeration("HydraulicNetworkElement::intPara_t", p.name);
					m_intPara[ptype] = p; success = true;
				}
				catch (...) { /* intentional fail */  }
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.name).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else if (cName == "ObservedPressureDiffElementIds")
				m_observedPressureDiffElementIds.setEncodedString(c->GetText());
			else if (cName == "HydraulicNetworkHeatExchange")
				m_heatExchange.readXML(c);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'HydraulicNetworkElement' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'HydraulicNetworkElement' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * HydraulicNetworkElement::writeXML(TiXmlElement * parent) const {
	if (m_id == NANDRAD::INVALID_ID)  return nullptr;
	TiXmlElement * e = new TiXmlElement("HydraulicNetworkElement");
	parent->LinkEndChild(e);

	if (m_id != NANDRAD::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<IDType>(m_id));
	if (m_inletNodeId != NANDRAD::INVALID_ID)
		e->SetAttribute("inletNodeId", IBK::val2string<IDType>(m_inletNodeId));
	if (m_outletNodeId != NANDRAD::INVALID_ID)
		e->SetAttribute("outletNodeId", IBK::val2string<IDType>(m_outletNodeId));
	if (m_inletZoneId != NANDRAD::INVALID_ID)
		e->SetAttribute("inletZoneId", IBK::val2string<IDType>(m_inletZoneId));
	if (m_outletZoneId != NANDRAD::INVALID_ID)
		e->SetAttribute("outletZoneId", IBK::val2string<IDType>(m_outletZoneId));
	if (m_componentId != NANDRAD::INVALID_ID)
		e->SetAttribute("componentId", IBK::val2string<IDType>(m_componentId));
	if (m_pipePropertiesId != NANDRAD::INVALID_ID)
		e->SetAttribute("pipePropertiesId", IBK::val2string<IDType>(m_pipePropertiesId));
	if (m_controlElementId != NANDRAD::INVALID_ID)
		e->SetAttribute("controlElementId", IBK::val2string<IDType>(m_controlElementId));
	if (!m_displayName.empty())
		e->SetAttribute("displayName", m_displayName);

	for (unsigned int i=0; i<NUM_P; ++i) {
		if (!m_para[i].name.empty()) {
			TiXmlElement::appendIBKParameterElement(e, m_para[i].name, m_para[i].IO_unit.name(), m_para[i].get_value(m_para[i].IO_unit));
		}
	}

	for (unsigned int i=0; i<NUM_IP; ++i) {
		if (!m_intPara[i].name.empty()) {
			TiXmlElement::appendSingleAttributeElement(e, "IBK:IntPara", "name", m_intPara[i].name, IBK::val2string(m_intPara[i].value));
		}
	}

	m_heatExchange.writeXML(e);
	if (!m_observedPressureDiffElementIds.m_values.empty())
		TiXmlElement::appendSingleAttributeElement(e, "ObservedPressureDiffElementIds", nullptr, std::string(), m_observedPressureDiffElementIds.encodedString());
	return e;
}

} // namespace NANDRAD
