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

#include <VICUS_Network.h>
#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <VICUS_Constants.h>
#include <NANDRAD_Utilities.h>
#include <VICUS_Constants.h>

#include <tinyxml.h>

namespace VICUS {

void Network::readXML(const TiXmlElement * element) {
	FUNCID(Network::readXML);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'id' attribute.") ), FUNC_ID);

		if (!TiXmlAttribute::attributeByName(element, "fluidID"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'fluidID' attribute.") ), FUNC_ID);

		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "fluidID")
				m_fluidID = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "name")
				m_name = attrib->ValueStr();
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
			if (cName == "Nodes") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "NetworkNode")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					NetworkNode obj;
					obj.readXML(c2);
					m_nodes.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Edges") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "NetworkEdge")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					NetworkEdge obj;
					obj.readXML(c2);
					m_edges.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Network' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Network' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * Network::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("Network");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (m_fluidID != VICUS::INVALID_ID)
		e->SetAttribute("fluidID", IBK::val2string<unsigned int>(m_fluidID));
	if (!m_name.empty())
		e->SetAttribute("name", m_name);

	if (!m_nodes.empty()) {
		TiXmlElement * child = new TiXmlElement("Nodes");
		e->LinkEndChild(child);

		for (std::vector<NetworkNode>::const_iterator it = m_nodes.begin();
			it != m_nodes.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_edges.empty()) {
		TiXmlElement * child = new TiXmlElement("Edges");
		e->LinkEndChild(child);

		for (std::vector<NetworkEdge>::const_iterator it = m_edges.begin();
			it != m_edges.end(); ++it)
		{
			it->writeXML(child);
		}
	}

	return e;
}

} // namespace VICUS
