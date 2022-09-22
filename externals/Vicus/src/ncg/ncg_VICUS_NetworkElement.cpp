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

#include <VICUS_NetworkElement.h>
#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <VICUS_Constants.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace VICUS {

void NetworkElement::readXML(const TiXmlElement * element) {
	FUNCID(NetworkElement::readXML);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'id' attribute.") ), FUNC_ID);

		if (!TiXmlAttribute::attributeByName(element, "inletNodeId"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'inletNodeId' attribute.") ), FUNC_ID);

		if (!TiXmlAttribute::attributeByName(element, "outletNodeId"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'outletNodeId' attribute.") ), FUNC_ID);

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
			else if (attribName == "componentId")
				m_componentId = (IDType)NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "controlElementId")
				m_controlElementId = (IDType)NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "displayName")
				m_displayName = QString::fromStdString(attrib->ValueStr());
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'NetworkElement' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'NetworkElement' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * NetworkElement::writeXML(TiXmlElement * parent) const {
	if (m_id == VICUS::INVALID_ID)  return nullptr;
	TiXmlElement * e = new TiXmlElement("NetworkElement");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<IDType>(m_id));
	if (m_inletNodeId != VICUS::INVALID_ID)
		e->SetAttribute("inletNodeId", IBK::val2string<IDType>(m_inletNodeId));
	if (m_outletNodeId != VICUS::INVALID_ID)
		e->SetAttribute("outletNodeId", IBK::val2string<IDType>(m_outletNodeId));
	if (m_componentId != VICUS::INVALID_ID)
		e->SetAttribute("componentId", IBK::val2string<IDType>(m_componentId));
	if (m_controlElementId != VICUS::INVALID_ID)
		e->SetAttribute("controlElementId", IBK::val2string<IDType>(m_controlElementId));
	if (!m_displayName.isEmpty())
		e->SetAttribute("displayName", m_displayName.toStdString());
	return e;
}

} // namespace VICUS
