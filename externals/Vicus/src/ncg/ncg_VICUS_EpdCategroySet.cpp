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

#include <VICUS_EPDCategroySet.h>
#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <VICUS_Constants.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace VICUS {

void EPDCategroySet::readXML(const TiXmlElement * element) {
	FUNCID(EPDCategroySet::readXML);

	try {
		// search for mandatory attributes
		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "idCategoryA")
				m_idCategoryA = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "idCategoryB")
				m_idCategoryB = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "idCategoryC")
				m_idCategoryC = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "idCategoryD")
				m_idCategoryD = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'EPDCategroySet' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'EPDCategroySet' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * EPDCategroySet::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("EPDCategroySet");
	parent->LinkEndChild(e);

	if (m_idCategoryA != VICUS::INVALID_ID)
		e->SetAttribute("idCategoryA", IBK::val2string<unsigned int>(m_idCategoryA));
	if (m_idCategoryB != VICUS::INVALID_ID)
		e->SetAttribute("idCategoryB", IBK::val2string<unsigned int>(m_idCategoryB));
	if (m_idCategoryC != VICUS::INVALID_ID)
		e->SetAttribute("idCategoryC", IBK::val2string<unsigned int>(m_idCategoryC));
	if (m_idCategoryD != VICUS::INVALID_ID)
		e->SetAttribute("idCategoryD", IBK::val2string<unsigned int>(m_idCategoryD));
	return e;
}

} // namespace VICUS
