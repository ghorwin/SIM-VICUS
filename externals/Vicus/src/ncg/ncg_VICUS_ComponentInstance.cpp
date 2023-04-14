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

#include <VICUS_ComponentInstance.h>
#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <VICUS_Constants.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace VICUS {

void ComponentInstance::readXML(const TiXmlElement * element) {
	FUNCID(ComponentInstance::readXML);

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
				m_id = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "idComponent")
				m_idComponent = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "idAcousticComponent")
				m_idAcousticComponent = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "idSideASurface")
				m_idSideASurface = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "idSideBSurface")
				m_idSideBSurface = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
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
			if (cName == "IdSurfaceHeating")
				m_idSurfaceHeating = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "IdSurfaceHeatingControlZone")
				m_idSurfaceHeatingControlZone = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "IdSupplySystem")
				m_idSupplySystem = NANDRAD::readPODElement<unsigned int>(c, cName);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'ComponentInstance' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'ComponentInstance' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * ComponentInstance::writeXML(TiXmlElement * parent) const {
	if (m_id == VICUS::INVALID_ID)  return nullptr;
	TiXmlElement * e = new TiXmlElement("ComponentInstance");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (m_idComponent != VICUS::INVALID_ID)
		e->SetAttribute("idComponent", IBK::val2string<unsigned int>(m_idComponent));
	if (m_idAcousticComponent != VICUS::INVALID_ID)
		e->SetAttribute("idAcousticComponent", IBK::val2string<unsigned int>(m_idAcousticComponent));
	if (m_idSideASurface != VICUS::INVALID_ID)
		e->SetAttribute("idSideASurface", IBK::val2string<unsigned int>(m_idSideASurface));
	if (m_idSideBSurface != VICUS::INVALID_ID)
		e->SetAttribute("idSideBSurface", IBK::val2string<unsigned int>(m_idSideBSurface));
	if (m_idSurfaceHeating != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdSurfaceHeating", nullptr, std::string(), IBK::val2string<unsigned int>(m_idSurfaceHeating));
	if (m_idSurfaceHeatingControlZone != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdSurfaceHeatingControlZone", nullptr, std::string(), IBK::val2string<unsigned int>(m_idSurfaceHeatingControlZone));
	if (m_idSupplySystem != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdSupplySystem", nullptr, std::string(), IBK::val2string<unsigned int>(m_idSupplySystem));
	return e;
}

} // namespace VICUS
