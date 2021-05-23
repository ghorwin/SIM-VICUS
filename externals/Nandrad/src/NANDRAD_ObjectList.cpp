/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "NANDRAD_ObjectList.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_KeywordList.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>

#include <tinyxml.h>

namespace NANDRAD {

void ObjectList::readXML(const TiXmlElement * element) {
	FUNCID(ObjectList::readXML);

	// read base stuff
	readXMLPrivate(element);

	try {
		// read filter ID
		for (const TiXmlElement * c = element->FirstChildElement(); c; c = c->NextSiblingElement()) {
			// determine data based on element name
			std::string cname = c->Value();
			// location specification is found
			if (cname == "FilterID") {
				m_filterID.setEncodedString(c->GetText());
			}
			else if (cname == "ReferenceType") {
				if (!KeywordList::KeywordExists("ModelInputReference::referenceType_t", c->GetText()) ) {
					throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
						IBK::FormatString("Invalid or unknown keyword '"+std::string(c->GetText())+"'.") ), FUNC_ID);
				}
				m_referenceType = (ModelInputReference::referenceType_t)
						KeywordList::Enumeration("ModelInputReference::referenceType_t", c->GetText());
			}
		}
		// check if we have found a type filter
		if (m_referenceType == ModelInputReference::NUM_MRT) {
			throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing tag 'ReferenceType'.") ), FUNC_ID);
		}
		// need either ID filter or name filter
		// exception: Filter type location and filter type schedule need no id filter
		if (m_referenceType != ModelInputReference::MRT_LOCATION && m_referenceType != ModelInputReference::MRT_SCHEDULE) {
			if (m_filterID.empty()) {
				throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
					IBK::FormatString("'FilterID' tag is required.") ), FUNC_ID);
			}
		}
	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading 'ObjectList' element."), FUNC_ID);
	}
	catch (std::exception &ex2) {
		throw IBK::Exception(IBK::FormatString("%1\nError reading 'ObjectList' element.").arg(ex2.what()), FUNC_ID);
	}
}


TiXmlElement * ObjectList::writeXML(TiXmlElement * parent) const {

	// first basic code
	TiXmlElement * e = writeXMLPrivate(parent);

	if (!m_filterID.empty())
		TiXmlElement::appendSingleAttributeElement(e, "FilterID", nullptr, std::string(), m_filterID.encodedString());

	TiXmlElement::appendSingleAttributeElement(	e,  "ReferenceType", nullptr, std::string(),
												KeywordList::Keyword("ModelInputReference::referenceType_t", m_referenceType));

	return e;
}


} // namespace NANDRAD

