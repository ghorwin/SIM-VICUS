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

#include <VICUS_RotationMatrix.h>
#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <VICUS_Constants.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace VICUS {

void RotationMatrix::readXML(const TiXmlElement * element) {
	FUNCID(RotationMatrix::readXML);

	try {
		// search for mandatory elements
		if (!element->FirstChildElement("Wp"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'Wp' element.") ), FUNC_ID);

		if (!element->FirstChildElement("X"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'X' element.") ), FUNC_ID);

		if (!element->FirstChildElement("Y"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'Y' element.") ), FUNC_ID);

		if (!element->FirstChildElement("Z"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'Z' element.") ), FUNC_ID);

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "Wp")
				m_wp = NANDRAD::readPODElement<float>(c, cName);
			else if (cName == "X")
				m_x = NANDRAD::readPODElement<float>(c, cName);
			else if (cName == "Y")
				m_y = NANDRAD::readPODElement<float>(c, cName);
			else if (cName == "Z")
				m_z = NANDRAD::readPODElement<float>(c, cName);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'RotationMatrix' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'RotationMatrix' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * RotationMatrix::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("RotationMatrix");
	parent->LinkEndChild(e);

	TiXmlElement::appendSingleAttributeElement(e, "Wp", nullptr, std::string(), IBK::val2string<float>(m_wp));
	TiXmlElement::appendSingleAttributeElement(e, "X", nullptr, std::string(), IBK::val2string<float>(m_x));
	TiXmlElement::appendSingleAttributeElement(e, "Y", nullptr, std::string(), IBK::val2string<float>(m_y));
	TiXmlElement::appendSingleAttributeElement(e, "Z", nullptr, std::string(), IBK::val2string<float>(m_z));
	return e;
}

} // namespace VICUS
