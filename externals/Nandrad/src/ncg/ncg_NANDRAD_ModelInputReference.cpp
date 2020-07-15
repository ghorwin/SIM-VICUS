/*	The NANDRAD data model library.
	Copyright (c) 2012-now, Institut fuer Bauklimatik, TU Dresden, Germany

	Written by
	A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
	A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
	St. Vogelsang	<stefan.vogelsang -[at]- tu-dresden.de>
	All rights reserved.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#include <NANDRAD_ModelInputReference.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <NANDRAD_Constants.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace NANDRAD {

void ModelInputReference::readXML(const TiXmlElement * element) {
	FUNCID("ModelInputReference::readXML");

	try {
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "constant")
				m_constant = readPODAttributeValue<bool>(element, attrib);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'ModelInputReference' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'ModelInputReference' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * ModelInputReference::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("ModelInputReference");
	parent->LinkEndChild(e);

	e->SetAttribute("constant", IBK::val2string<bool>(m_constant));
	if (!m_targetName.empty())
		TiXmlElement::appendSingleAttributeElement(e, "TargetName", nullptr, std::string(), m_targetName);
	if (!m_objectList.empty())
		TiXmlElement::appendSingleAttributeElement(e, "ObjectList", nullptr, std::string(), m_objectList);
	if (!m_quantity.empty())
		TiXmlElement::appendSingleAttributeElement(e, "Quantity", nullptr, std::string(), m_quantity);
	return e;
}

} // namespace NANDRAD
