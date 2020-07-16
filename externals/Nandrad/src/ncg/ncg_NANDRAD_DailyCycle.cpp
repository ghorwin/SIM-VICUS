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

#include <NANDRAD_DailyCycle.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <NANDRAD_Constants.h>
#include <NANDRAD_KeywordList.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace NANDRAD {

void DailyCycle::readXML(const TiXmlElement * element) {
	FUNCID(DailyCycle::readXML);

	try {
		// search for mandatory attributes
		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "interpolation")
			try {
				m_interpolation = (interpolation_t)KeywordList::Enumeration("DailyCycle::interpolation_t", attrib->ValueStr());
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
					IBK::FormatString("Invalid or unknown keyword '"+attrib->ValueStr()+"'.") ), FUNC_ID);
			}
			attrib = attrib->Next();
		}
		// search for mandatory elements
		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "TimeUnit")
				m_timeUnit = readUnitElement(c, cName);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'DailyCycle' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'DailyCycle' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * DailyCycle::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("DailyCycle");
	parent->LinkEndChild(e);

	if (m_interpolation != NUM_IT)
		e->SetAttribute("interpolation", KeywordList::Keyword("DailyCycle::interpolation_t",  m_interpolation));

	if (!m_timePoints.empty()) {
		TiXmlElement * child = new TiXmlElement("TimePoints");
		e->LinkEndChild(child);

		std::stringstream vals;
		for (std::vector<double>::const_iterator ifaceIt = m_timePoints.begin();
			ifaceIt != m_timePoints.end(); ++ifaceIt)
		{
			vals << *ifaceIt;
		}
		TiXmlText * text = new TiXmlText( vals.str() );
		child->LinkEndChild( text );
	}

	if (m_timeUnit.id() != 0)
		TiXmlElement::appendSingleAttributeElement(e, "TimeUnit", nullptr, std::string(), m_timeUnit.name());

	m_values.writeXML(e);
	return e;
}

} // namespace NANDRAD
