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

#include <NANDRAD_WindowGlazingLayer.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <NANDRAD_Constants.h>
#include <NANDRAD_Constants.h>
#include <NANDRAD_KeywordList.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace NANDRAD {

void WindowGlazingLayer::readXML(const TiXmlElement * element) {
	FUNCID(WindowGlazingLayer::readXML);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "type"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'type' attribute.") ), FUNC_ID);

		if (!TiXmlAttribute::attributeByName(element, "id"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'id' attribute.") ), FUNC_ID);

		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "type")
			try {
				m_type = (type_t)KeywordList::Enumeration("WindowGlazingLayer::type_t", attrib->ValueStr());
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
					IBK::FormatString("Invalid or unknown keyword '"+attrib->ValueStr()+"'.") ), FUNC_ID);
			}
			if (attribName == "id")
				m_id = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
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
					ptype = (para_t)KeywordList::Enumeration("WindowGlazingLayer::para_t", p.name);
					m_para[ptype] = p;
					success = true;
				}
				catch (IBK::Exception & ex) { ex.writeMsgStackToError(); }
				if (success) {
					std::string refUnit = KeywordList::Unit("WindowGlazingLayer::para_t", ptype);
					if (!refUnit.empty() && (p.IO_unit.base_id() != IBK::Unit(refUnit).base_id())) {
						throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(c->Row())
											  .arg("Incompatible unit '"+p.IO_unit.name()+"', expected '"+refUnit +"'."), FUNC_ID);
					}
				}
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.name).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else if (cName == "LinearSplineParameter") {
				NANDRAD::LinearSplineParameter p;
				p.readXML(c);
				bool success = false;
				if (p.m_name == "Conductivity") {
					m_conductivity = p; success = true;
				}
				else if (p.m_name == "DynamicViscosity") {
					m_dynamicViscosity = p; success = true;
				}
				else if (p.m_name == "HeatCapacity") {
					m_heatCapacity = p; success = true;
				}
				else if (p.m_name == "ShortWaveTransmittance") {
					m_shortWaveTransmittance = p; success = true;
				}
				else if (p.m_name == "ShortWaveReflectanceInside") {
					m_shortWaveReflectanceInside = p; success = true;
				}
				else if (p.m_name == "ShortWaveReflectanceOutside") {
					m_shortWaveReflectanceOutside = p; success = true;
				}
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.m_name).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'WindowGlazingLayer' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'WindowGlazingLayer' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * WindowGlazingLayer::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("WindowGlazingLayer");
	parent->LinkEndChild(e);

	if (m_type != NUM_T)
		e->SetAttribute("type", KeywordList::Keyword("WindowGlazingLayer::type_t",  m_type));
	if (m_id != NANDRAD::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_displayName.empty())
		e->SetAttribute("displayName", m_displayName);

	for (unsigned int i=0; i<NUM_P; ++i) {
		if (!m_para[i].name.empty())
			TiXmlElement::appendIBKParameterElement(e, m_para[i].name, m_para[i].IO_unit.name(), m_para[i].get_value());
	}
	if (!m_conductivity.m_name.empty())
		m_conductivity.writeXML(e);
	if (!m_dynamicViscosity.m_name.empty())
		m_dynamicViscosity.writeXML(e);
	if (!m_heatCapacity.m_name.empty())
		m_heatCapacity.writeXML(e);
	if (!m_shortWaveTransmittance.m_name.empty())
		m_shortWaveTransmittance.writeXML(e);
	if (!m_shortWaveReflectanceInside.m_name.empty())
		m_shortWaveReflectanceInside.writeXML(e);
	if (!m_shortWaveReflectanceOutside.m_name.empty())
		m_shortWaveReflectanceOutside.writeXML(e);
	return e;
}

} // namespace NANDRAD
