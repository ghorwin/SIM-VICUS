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

#include "NANDRAD_Constants.h"
#include "NANDRAD_ProjectInfo.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>

#include <tinyxml.h>

#include "NANDRAD_KeywordList.h"

namespace NANDRAD {


void ProjectInfo::readXML(const TiXmlElement * element) {
#if 0
	const char * const FUNC_ID = "[ProjectInfo::readXML]";

	// read optional attributes
	const TiXmlAttribute * attrib = TiXmlAttribute::attributeByName(element, "version");
	if (attrib != nullptr)
		m_version = attrib->Value();

	attrib = TiXmlAttribute::attributeByName(element, "created");
	if (attrib != nullptr)
		m_created = attrib->Value();

	attrib = TiXmlAttribute::attributeByName(element, "lastEdited");
	if (attrib != nullptr)
		m_lastEdited = attrib->Value();

	// loop over all elements in this XML element
	for (const TiXmlElement * e = element->FirstChildElement(); e; e = e->NextSiblingElement()) {

		// get element name
		std::string name = e->Value();

		// handle known elements
		if (name == "Comment") {
			m_comment = e->GetText();
		}
		else if (name == "GridParameter") {

			for (const TiXmlElement * gp = e->FirstChildElement(); gp; gp = gp->NextSiblingElement()) {

				// get element name
				std::string ename = gp->Value();

				if (ename == "IBK:Parameter") {
					// use utility function to read parameter
					std::string namestr, unitstr;
					double value;
					TiXmlElement::readIBKParameterElement( gp, namestr, unitstr, value);
					// determine type of parameter
					para_t t = (para_t)KeywordList::Enumeration("ProjectInfo::para_t", namestr);
					m_para[t].set(namestr, value, unitstr);

					// check parameter unit
					std::string paraUnit = KeywordList::Unit("ProjectInfo::para_t", t);
					if (unitstr != paraUnit) {
						try {
							m_para[t].get_value(paraUnit);
						}
						catch (IBK::Exception &ex) {
							throw IBK::Exception(ex, IBK::FormatString(XML_READ_ERROR).arg(gp->Row()).arg(
								IBK::FormatString("Invalid unit '#%1' of parameter #%2!")
								.arg(paraUnit)
								.arg(namestr)
								), FUNC_ID);
						}
					}
				}
				else if (ename == "IBK:Flag") {
					// use utility function to read parameter
					std::string namestr;
					std::string flag;
					TiXmlElement::readSingleAttributeElement( gp, "name", namestr, flag );
					// determine type of flag
					flag_t t = (flag_t)KeywordList::Enumeration("ProjectInfo::flag_t", namestr);
					m_flag[t].set(namestr, (flag == "true" || flag == "1"));
				}
				else {
					throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(gp->Row()).arg(
						IBK::FormatString("Unknown XML tag with name '%1' in GridParameter element.").arg(ename)
						), FUNC_ID);
				}
			}

		}
		else {
			throw IBK::Exception(IBK::FormatString(
					"Unknown element '%1' in ProjectInfo section. Please check line: %2 ").arg(name).arg(e->Row()), FUNC_ID );
		}
	}
#endif
}
// ----------------------------------------------------------------------------


void ProjectInfo::writeXML(TiXmlElement * parent) const {
#if 0
	TiXmlComment::addComment(parent,
		"General Project Information (author, date, project ID etc.)");

	TiXmlElement * e1 = new TiXmlElement( "ProjectInfo" );
	parent->LinkEndChild( e1 );

	// write in current project file version
	e1->SetAttribute("version", LONG_VERSION);
	if (!m_created.empty())
		e1->SetAttribute("created", m_created);
	if (!m_lastEdited.empty())
		e1->SetAttribute("lastEdited", m_lastEdited);

	if (!m_comment.empty())
		TiXmlElement::appendSingleAttributeElement(e1, "Comment", nullptr, std::string(), m_comment);

	TiXmlElement * e2 = new TiXmlElement( "GridParameter" );
	e1->LinkEndChild( e2 );

	for (int i=0; i<NUM_P; ++i) {
		if (m_para[i].name.empty()) continue;
		TiXmlElement::appendIBKParameterElement(e2, m_para[i].name, m_para[i].IO_unit.name(), m_para[i].get_value());
	}
	for (int i=0; i<NUM_F; ++i) {
		if (m_flag[i].name().empty()) continue;
		TiXmlElement::appendSingleAttributeElement(e2, "IBK:Flag", "name", m_flag[i].name(),
												   m_flag[i].isEnabled() ? "true" : "false");
	}

	TiXmlComment::addSeparatorComment(parent);
#endif
}
// ----------------------------------------------------------------------------

} // namespace NANDRAD

