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

#include <VICUS_NetworkEdge.h>
#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <VICUS_Constants.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace VICUS {

void NetworkEdge::readXML(const TiXmlElement * element) {
	FUNCID(NetworkEdge::readXML);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "idNode1"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'idNode1' attribute.") ), FUNC_ID);

		if (!TiXmlAttribute::attributeByName(element, "idNode2"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'idNode2' attribute.") ), FUNC_ID);

		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "supply")
				m_supply = NANDRAD::readPODAttributeValue<bool>(element, attrib);
			else if (attribName == "idPipe")
				m_idPipe = (IDType)NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "displayName")
				m_displayName = QString::fromStdString(attrib->ValueStr());
			else if (attribName == "visible")
				m_visible = NANDRAD::readPODAttributeValue<bool>(element, attrib);
			else if (attribName == "idNode1")
				m_idNode1 = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "idNode2")
				m_idNode2 = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
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
			if (cName == "Length")
				m_length = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "HydraulicNetworkHeatExchange")
				m_heatExchange.readXML(c);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'NetworkEdge' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'NetworkEdge' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * NetworkEdge::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("NetworkEdge");
	parent->LinkEndChild(e);

	if (m_supply != NetworkEdge().m_supply)
			e->SetAttribute("supply", "true");
	if (m_idPipe != VICUS::INVALID_ID)
		e->SetAttribute("idPipe", IBK::val2string<IDType>(m_idPipe));
	if (!m_displayName.isEmpty())
		e->SetAttribute("displayName", m_displayName.toStdString());
	if (m_visible != NetworkEdge().m_visible)
			e->SetAttribute("visible", "true");
	if (m_idNode1 != VICUS::INVALID_ID)
		e->SetAttribute("idNode1", IBK::val2string<unsigned int>(m_idNode1));
	if (m_idNode2 != VICUS::INVALID_ID)
		e->SetAttribute("idNode2", IBK::val2string<unsigned int>(m_idNode2));

	m_heatExchange.writeXML(e);
	TiXmlElement::appendSingleAttributeElement(e, "Length", nullptr, std::string(), IBK::val2string<double>(m_length));
	return e;
}

} // namespace VICUS
