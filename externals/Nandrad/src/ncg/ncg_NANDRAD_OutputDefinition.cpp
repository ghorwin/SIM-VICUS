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

#include <NANDRAD_OutputDefinition.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <NANDRAD_Constants.h>
#include <NANDRAD_KeywordList.h>

#include <tinyxml.h>

namespace NANDRAD {

void OutputDefinition::readXML(const TiXmlElement * element) {
	FUNCID(OutputDefinition::readXML);

	try {
		// search for mandatory elements
		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "Filename")
				m_filename = c->GetText();
			else if (cName == "Quantity")
				m_quantity = c->GetText();
			else if (cName == "ObjectListName")
				m_objectListName = c->GetText();
			else if (cName == "GridName")
				m_gridName = c->GetText();
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'OutputDefinition' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'OutputDefinition' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * OutputDefinition::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("OutputDefinition");
	parent->LinkEndChild(e);

	if (!m_filename.empty())
		TiXmlElement::appendSingleAttributeElement(e, "Filename", nullptr, std::string(), m_filename);
	if (!m_quantity.empty())
		TiXmlElement::appendSingleAttributeElement(e, "Quantity", nullptr, std::string(), m_quantity);

	if (m_timeType != NUM_OTT)
		TiXmlElement::appendSingleAttributeElement(e, "TimeType", nullptr, std::string(), KeywordList::Keyword("OutputDefinition::timeType_t",  m_timeType));
	if (!m_objectListName.empty())
		TiXmlElement::appendSingleAttributeElement(e, "ObjectListName", nullptr, std::string(), m_objectListName);
	if (!m_gridName.empty())
		TiXmlElement::appendSingleAttributeElement(e, "GridName", nullptr, std::string(), m_gridName);
	return e;
}

} // namespace NANDRAD
