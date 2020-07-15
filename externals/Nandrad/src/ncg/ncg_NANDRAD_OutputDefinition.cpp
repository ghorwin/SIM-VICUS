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
	FUNCID("OutputDefinition::readXML");

	try {
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

	TiXmlElement::appendSingleAttributeElement(e, "Filename", nullptr, std::string(), m_filename);
	TiXmlElement::appendSingleAttributeElement(e, "Quantity", nullptr, std::string(), m_quantity);

	if (m_timeType != NUM_OTT)
		TiXmlElement::appendSingleAttributeElement(e, "TimeType", nullptr, std::string(), KeywordList::Keyword("OutputDefinition::timeType_t",  m_timeType));
	TiXmlElement::appendSingleAttributeElement(e, "ObjectListName", nullptr, std::string(), m_objectListName);
	TiXmlElement::appendSingleAttributeElement(e, "GridName", nullptr, std::string(), m_gridName);
	return e;
}

} // namespace NANDRAD
