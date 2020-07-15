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

#include <NANDRAD_ProjectInfo.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <NANDRAD_Constants.h>

#include <tinyxml.h>

namespace NANDRAD {

void ProjectInfo::readXML(const TiXmlElement * element) {
	FUNCID("ProjectInfo::readXML");

	try {
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "Comment")
				m_comment = c->GetText();
			else if (cName == "Created")
				m_created = c->GetText();
			else if (cName == "LastEdited")
				m_lastEdited = c->GetText();
			else if (cName == "Version")
				m_version = c->GetText();
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'ProjectInfo' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'ProjectInfo' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * ProjectInfo::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("ProjectInfo");
	parent->LinkEndChild(e);


	TiXmlElement::appendSingleAttributeElement(e, "Comment", nullptr, std::string(), m_comment);

	TiXmlElement::appendSingleAttributeElement(e, "Created", nullptr, std::string(), m_created);

	TiXmlElement::appendSingleAttributeElement(e, "LastEdited", nullptr, std::string(), m_lastEdited);

	TiXmlElement::appendSingleAttributeElement(e, "Version", nullptr, std::string(), m_version);
	return e;
}

} // namespace NANDRAD
