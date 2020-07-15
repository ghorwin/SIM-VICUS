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

#include "NANDRAD_ObjectList.h"
#include "NANDRAD_FindHelpers.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_KeywordList.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>

#include <tinyxml.h>

namespace NANDRAD {

void ObjectList::readXML(const TiXmlElement * element) {
	FUNCID("ObjectList::readXML");

	// read base stuff
	readXMLPrivate(element);
#if 0
	try {
		// read filter ID
		for (const TiXmlElement * c = element->FirstChildElement(); c; c = c->NextSiblingElement()) {
			// determine data based on element name
			std::string cname = c->Value();
			// location specification is found
			if (cname == "FilterID") {
				m_filterID.setEncodedString(c->GetText());
			}
			else {
				throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
									IBK::FormatString("Unknown XML tag with name '%1'.").arg(cname)
										), FUNC_ID);
			}
		}
		// check if we have found a type filter
		if (m_filterType == ModelInputReference::NUM_MRT)
		{
			throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing tag 'FilterType'.")
				), FUNC_ID);
		}
		// need either ID filter or name filter
		// exception: Filter type location and filter type schedule need no id filter
		if(m_filterType != ModelInputReference::MRT_LOCATION && m_filterType != ModelInputReference::MRT_SCHEDULE)
		{
			if (m_filterID.empty() && m_filterDisplayName.empty() && m_filterSpaceType.empty())
			{
				throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
					IBK::FormatString("Neither Tag 'FilterID', 'FilterSpaceType' nor FilterDisplayName' is provided.")
					), FUNC_ID);
			}
		}
	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading 'ObjectList' element."), FUNC_ID);
	}
	catch (std::exception &ex2) {
		throw IBK::Exception(IBK::FormatString("%1\nError reading 'ObjectList' element.").arg(ex2.what()), FUNC_ID);
	}
#endif
}


TiXmlElement * ObjectList::writeXML(TiXmlElement * parent) const {

	// first basic code
	TiXmlElement * e = writeXMLPrivate(parent);

	// now write custom stuff
#if 0
	TiXmlElement * e = new TiXmlElement("ObjectList");
	parent->LinkEndChild(e);

	e->SetAttribute("name",m_name);
	TiXmlElement::appendSingleAttributeElement(	e,
												"FilterType",
												nullptr, std::string(),
												KeywordList::Keyword("ModelInputReference::referenceType_t",m_filterType));
	if (!m_filterID.empty()) {
		TiXmlElement::appendSingleAttributeElement(	e,
													"FilterID",
													nullptr, std::string(),
													m_filterID.encodedString());
	}
	if (!m_filterSpaceType.empty()) {
		std::string spaceTypeString;
		for (unsigned int i = 0; i < m_filterSpaceType.size(); ++i) {
			if (i > 0)
				spaceTypeString += std::string(",");
			spaceTypeString += m_filterSpaceType[i];
		}
		TiXmlElement::appendSingleAttributeElement(	e,
													"FilterSpaceType",
													nullptr, std::string(),
													spaceTypeString);
	}
	if (!m_filterDisplayName.empty()) {
		if(detailedOutput)
			TiXmlComment::addComment(e,"Regular Expressions that are used for displayName string comparison.");
		TiXmlElement::appendSingleAttributeElement(	e,
													"FilterDisplayName",
													nullptr, std::string(),
													m_filterDisplayName);
	}
#endif
	return e;
}


} // namespace NANDRAD

