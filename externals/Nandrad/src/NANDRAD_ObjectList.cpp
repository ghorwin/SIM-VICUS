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

ObjectList::ObjectList()
{
}


void ObjectList::readXML(const TiXmlElement * element) {
#if 0
	const char * const FUNC_ID = "[ObjectList::readXML]";

	// read outputgrid name
	try {
		const TiXmlAttribute * attrib = TiXmlAttribute::attributeByName(element, "name");
		if (!attrib)
			throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing 'name' attribute.")
				), FUNC_ID);

		m_name = attrib->Value();

		// read sub-elements
		for (const TiXmlElement * c = element->FirstChildElement(); c; c = c->NextSiblingElement()) {
			// determine data based on element name
			std::string cname = c->Value();
			if(cname == "FilterType") {
				if(!KeywordList::KeywordExists("ModelInputReference::referenceType_t", c->GetText()) )
				{
					throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
						IBK::FormatString("Unknown FilterType '%1'.").arg(c->GetText())
						), FUNC_ID);
				}
				m_filterType = (ModelInputReference::referenceType_t)
						KeywordList::Enumeration("ModelInputReference::referenceType_t", c->GetText());
			}
			// location specification is found
			else if(cname == "FilterID") {
				m_filterID.setEncodedString(c->GetText());
			}
			// space type specification is found
			else if(cname == "FilterSpaceType") {
				// space types are devided by ','
				IBK::explode(c->GetText(),m_filterSpaceType,',',true);
			}
			// location specification is found
			else if(cname == "FilterDisplayName") {
				m_filterDisplayName = c->GetText();
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


void ObjectList::writeXML(TiXmlElement * parent) const {
#if 0
	// write output definition
	if(detailedOutput)
		TiXmlComment::addComment(parent,IBK::FormatString("Specify all %1s that belong to '%2'.")
							.arg(KeywordList::Keyword("ModelInputReference::referenceType_t", m_filterType) )
							.arg(m_name)
							.str());
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
}


} // namespace NANDRAD

