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

#include <NANDRAD_Interface.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <NANDRAD_Constants.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace NANDRAD {

void Interface::readXMLPrivate(const TiXmlElement * element) {
	FUNCID(Interface::readXMLPrivate);

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
				m_id = readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "zoneId")
				m_zoneId = readPODAttributeValue<unsigned int>(element, attrib);
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
			if (cName == "InterfaceHeatConduction")
				m_heatConduction.readXML(c);
			else if (cName == "InterfaceSolarAbsorption")
				m_solarAbsorption.readXML(c);
			else if (cName == "InterfaceLongWaveEmission")
				m_longWaveEmission.readXML(c);
			else if (cName == "InterfaceVaporDiffusion")
				m_vaporDiffusion.readXML(c);
			else if (cName == "InterfaceAirFlow")
				m_airFlow.readXML(c);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Interface' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Interface' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * Interface::writeXMLPrivate(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("Interface");
	parent->LinkEndChild(e);

	TiXmlComment * com = new TiXmlComment();
	com->SetValue(m_comment);
	e->LinkEndChild(com);

	e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	e->SetAttribute("zoneId", IBK::val2string<unsigned int>(m_zoneId));

	m_heatConduction.writeXML(e);

	m_solarAbsorption.writeXML(e);

	m_longWaveEmission.writeXML(e);

	m_vaporDiffusion.writeXML(e);

	m_airFlow.writeXML(e);
	return e;
}

} // namespace NANDRAD
