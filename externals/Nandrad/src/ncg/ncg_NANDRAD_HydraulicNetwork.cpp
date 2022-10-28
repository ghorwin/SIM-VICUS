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

#include <NANDRAD_HydraulicNetwork.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <NANDRAD_Constants.h>
#include <NANDRAD_KeywordList.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace NANDRAD {

void HydraulicNetwork::readXMLPrivate(const TiXmlElement * element) {
	FUNCID(HydraulicNetwork::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'id' attribute.") ), FUNC_ID);

		if (!TiXmlAttribute::attributeByName(element, "modelType"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'modelType' attribute.") ), FUNC_ID);

		if (!TiXmlAttribute::attributeByName(element, "referenceElementId"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'referenceElementId' attribute.") ), FUNC_ID);

		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "displayName")
				m_displayName = attrib->ValueStr();
			else if (attribName == "modelType")
				try {
					m_modelType = (ModelType)KeywordList::Enumeration("HydraulicNetwork::ModelType", attrib->ValueStr());
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
						IBK::FormatString("Invalid or unknown keyword '"+attrib->ValueStr()+"'.") ), FUNC_ID);
				}
			else if (attribName == "referenceElementId")
				m_referenceElementId = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}
		// search for mandatory elements
		if (!element->FirstChildElement("HydraulicFluid"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'HydraulicFluid' element.") ), FUNC_ID);

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
					ptype = (para_t)KeywordList::Enumeration("HydraulicNetwork::para_t", p.name);
					m_para[ptype] = p; success = true;
				}
				catch (...) { /* intentional fail */  }
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.name).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else if (cName == "PipeProperties") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "HydraulicNetworkPipeProperties")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					HydraulicNetworkPipeProperties obj;
					obj.readXML(c2);
					m_pipeProperties.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Components") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "HydraulicNetworkComponent")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					HydraulicNetworkComponent obj;
					obj.readXML(c2);
					m_components.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Nodes") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "HydraulicNetworkNode")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					HydraulicNetworkNode obj;
					obj.readXML(c2);
					m_nodes.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Elements") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "HydraulicNetworkElement")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					HydraulicNetworkElement obj;
					obj.readXML(c2);
					m_elements.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "ControlElements") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "HydraulicNetworkControlElement")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					HydraulicNetworkControlElement obj;
					obj.readXML(c2);
					m_controlElements.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "HydraulicFluid")
				m_fluid.readXML(c);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'HydraulicNetwork' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'HydraulicNetwork' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * HydraulicNetwork::writeXMLPrivate(TiXmlElement * parent) const {
	if (m_id == NANDRAD::INVALID_ID)  return nullptr;
	TiXmlElement * e = new TiXmlElement("HydraulicNetwork");
	parent->LinkEndChild(e);

	if (m_id != NANDRAD::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_displayName.empty())
		e->SetAttribute("displayName", m_displayName);
	if (m_modelType != NUM_MT)
		e->SetAttribute("modelType", KeywordList::Keyword("HydraulicNetwork::ModelType",  m_modelType));
	if (m_referenceElementId != NANDRAD::INVALID_ID)
		e->SetAttribute("referenceElementId", IBK::val2string<unsigned int>(m_referenceElementId));

	m_fluid.writeXML(e);

	for (unsigned int i=0; i<NUM_P; ++i) {
		if (!m_para[i].name.empty()) {
			TiXmlElement::appendIBKParameterElement(e, m_para[i].name, m_para[i].IO_unit.name(), m_para[i].get_value(m_para[i].IO_unit));
		}
	}

	if (!m_pipeProperties.empty()) {
		TiXmlElement * child = new TiXmlElement("PipeProperties");
		e->LinkEndChild(child);

		for (std::vector<HydraulicNetworkPipeProperties>::const_iterator it = m_pipeProperties.begin();
			it != m_pipeProperties.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_components.empty()) {
		TiXmlElement * child = new TiXmlElement("Components");
		e->LinkEndChild(child);

		for (std::vector<HydraulicNetworkComponent>::const_iterator it = m_components.begin();
			it != m_components.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_nodes.empty()) {
		TiXmlElement * child = new TiXmlElement("Nodes");
		e->LinkEndChild(child);

		for (std::vector<HydraulicNetworkNode>::const_iterator it = m_nodes.begin();
			it != m_nodes.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_elements.empty()) {
		TiXmlElement * child = new TiXmlElement("Elements");
		e->LinkEndChild(child);

		for (std::vector<HydraulicNetworkElement>::const_iterator it = m_elements.begin();
			it != m_elements.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_controlElements.empty()) {
		TiXmlElement * child = new TiXmlElement("ControlElements");
		e->LinkEndChild(child);

		for (std::vector<HydraulicNetworkControlElement>::const_iterator it = m_controlElements.begin();
			it != m_controlElements.end(); ++it)
		{
			it->writeXML(child);
		}
	}

	return e;
}

} // namespace NANDRAD
