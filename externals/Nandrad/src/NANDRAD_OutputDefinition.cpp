/*	The NANDRAD data model library.
Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

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

#include <algorithm>
#include <numeric>

#include "NANDRAD_OutputDefinition.h"
#include "NANDRAD_FindHelpers.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_KeywordList.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>

#include <tinyxml.h>

namespace NANDRAD {


OutputDefinition::OutputDefinition():
	m_fileType(FT_DATAIO),
	m_timeType(OTT_NONE),
	m_objectListRef(NULL)
{
}


void OutputDefinition::readXML(const TiXmlElement * element) {
	const char * const FUNC_ID = "[OutputDefinition::readXML]";

	try {
		// read sub-elements
		for (const TiXmlElement * c = element->FirstChildElement(); c; c = c->NextSiblingElement()) {
			// determine data based on element name
			std::string cname = c->Value();
			if (cname == "OutputGridName") {
				m_gridName = c->GetText();
			}
			else if (cname == "QuantityName") {
				m_quantityName = c->GetText();
			}
			else if (cname == "FileType") {
				if (!KeywordList::KeywordExists("OutputDefinition::fileType_t", c->GetText())) {
					throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
						IBK::FormatString("Invalid file type '%1'.").arg(c->GetText())
					), FUNC_ID);
				}

				m_fileType = (fileType_t)KeywordList::Enumeration("OutputDefinition::fileType_t", c->GetText());
			}
			else if (cname == "TimeType") {
				if(!KeywordList::KeywordExists("OutputDefinition::timeType_t", c->GetText())) {
					throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
						IBK::FormatString("Invalid time type '%1'.").arg(c->GetText())
						), FUNC_ID);
				}

				m_timeType = (timeType_t) KeywordList::Enumeration("OutputDefinition::timeType_t", c->GetText());
			}
			else if (cname == "TimeUnit") {
				m_timeUnit = c->GetText();
			}
			else if (cname == "ObjectListName") {
				m_objectListName = c->GetText();
			}
			else if (cname == "Quantity") {
				m_quantity = c->GetText();
			}
			else {
				throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
					IBK::FormatString("Unknown XML tag '%1'.").arg(cname)
					), FUNC_ID);
			}
		}
	}
	catch(IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading 'OutputDefinition' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception(IBK::FormatString("%1\nError reading 'OutputDefinition' element.").arg(ex2.what()), FUNC_ID);
	}
}


void OutputDefinition::writeXML(TiXmlElement * parent) const {
	OutputDefinition tmp;
	if (tmp == *this)
		return;
	// write output definition
	TiXmlElement * e = new TiXmlElement("OutputDefinition");
	parent->LinkEndChild(e);

	if (!m_gridName.empty())
		TiXmlElement::appendSingleAttributeElement(	e, "OutputGridName", NULL, std::string(), m_gridName);

	if (!m_quantityName.empty()) {
		TiXmlElement::appendSingleAttributeElement(e, "QuantityName", NULL, std::string(), m_quantityName);
	}
	if (m_fileType != FT_DATAIO)
		TiXmlElement::appendSingleAttributeElement(e, "FileType", NULL, std::string(),
			KeywordList::Keyword("OutputDefinition::fileType_t", m_fileType));
	if (m_timeType != OTT_NONE)
		TiXmlElement::appendSingleAttributeElement(	e, "TimeType", NULL, std::string(),
													KeywordList::Keyword("OutputDefinition::timeType_t",m_timeType));
	if (!m_timeUnit.empty())
		TiXmlElement::appendSingleAttributeElement(e, "TimeUnit", NULL, std::string(), m_timeUnit);

	if (!m_objectListName.empty())
		TiXmlElement::appendSingleAttributeElement(	e, "ObjectListName", NULL, std::string(), m_objectListName);
	if (!m_quantity.empty())
		TiXmlElement::appendSingleAttributeElement(	e, "Quantity", NULL, std::string(), m_quantity);
}


bool OutputDefinition::operator!=(const OutputDefinition & other) const {
	return (m_gridName != other.m_gridName ||
			m_timeType != other.m_timeType ||
			m_objectListName != other.m_objectListName ||
			m_quantity != other.m_quantity);
}

} // namespace NANDRAD

