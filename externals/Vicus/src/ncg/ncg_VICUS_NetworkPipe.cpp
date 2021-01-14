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

#include <VICUS_NetworkPipe.h>
#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <VICUS_Constants.h>
#include <NANDRAD_Utilities.h>
#include <VICUS_Constants.h>

#include <tinyxml.h>

namespace VICUS {

void NetworkPipe::readXML(const TiXmlElement * element) {
	FUNCID(NetworkPipe::readXML);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'id' attribute.") ), FUNC_ID);

		if (!TiXmlAttribute::attributeByName(element, "diameterOutside"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'diameterOutside' attribute.") ), FUNC_ID);

		if (!TiXmlAttribute::attributeByName(element, "wallThickness"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'wallThickness' attribute.") ), FUNC_ID);

		if (!TiXmlAttribute::attributeByName(element, "lambdaWall"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'lambdaWall' attribute.") ), FUNC_ID);

		if (!TiXmlAttribute::attributeByName(element, "roughness"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'roughness' attribute.") ), FUNC_ID);

		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "displayName")
				m_displayName.setEncodedString(attrib->ValueStr());
			else if (attribName == "diameterOutside")
				m_diameterOutside = NANDRAD::readPODAttributeValue<double>(element, attrib);
			else if (attribName == "wallThickness")
				m_wallThickness = NANDRAD::readPODAttributeValue<double>(element, attrib);
			else if (attribName == "lambdaWall")
				m_lambdaWall = NANDRAD::readPODAttributeValue<double>(element, attrib);
			else if (attribName == "roughness")
				m_roughness = NANDRAD::readPODAttributeValue<double>(element, attrib);
			else if (attribName == "insulationThickness")
				m_insulationThickness = NANDRAD::readPODAttributeValue<double>(element, attrib);
			else if (attribName == "lambdaInsulation")
				m_lambdaInsulation = NANDRAD::readPODAttributeValue<double>(element, attrib);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'NetworkPipe' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'NetworkPipe' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * NetworkPipe::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("NetworkPipe");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_displayName.empty())
		e->SetAttribute("displayName", m_displayName.encodedString());
	e->SetAttribute("diameterOutside", IBK::val2string<double>(m_diameterOutside));
	e->SetAttribute("wallThickness", IBK::val2string<double>(m_wallThickness));
	e->SetAttribute("lambdaWall", IBK::val2string<double>(m_lambdaWall));
	e->SetAttribute("roughness", IBK::val2string<double>(m_roughness));
	e->SetAttribute("insulationThickness", IBK::val2string<double>(m_insulationThickness));
	e->SetAttribute("lambdaInsulation", IBK::val2string<double>(m_lambdaInsulation));
	return e;
}

} // namespace VICUS
