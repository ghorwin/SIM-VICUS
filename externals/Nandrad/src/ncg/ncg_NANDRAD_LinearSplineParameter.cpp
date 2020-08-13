/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#include <NANDRAD_LinearSplineParameter.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <NANDRAD_Constants.h>
#include <NANDRAD_KeywordList.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace NANDRAD {

void LinearSplineParameter::readXML(const TiXmlElement * element) {
	FUNCID(LinearSplineParameter::readXML);

	try {
		// search for mandatory attributes
		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "name")
				m_name = attrib->ValueStr();
			else if (attribName == "interpolationMethod")
			try {
				m_interpolationMethod = (interpolationMethod_t)KeywordList::Enumeration("LinearSplineParameter::interpolationMethod_t", attrib->ValueStr());
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
					IBK::FormatString("Invalid or unknown keyword '"+attrib->ValueStr()+"'.") ), FUNC_ID);
			}
			else if (attribName == "wrapMethod")
			try {
				m_wrapMethod = (wrapMethod_t)KeywordList::Enumeration("LinearSplineParameter::wrapMethod_t", attrib->ValueStr());
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
					IBK::FormatString("Invalid or unknown keyword '"+attrib->ValueStr()+"'.") ), FUNC_ID);
			}
			else if (attribName == "xUnit")
				try {
					m_xUnit = IBK::Unit(attrib->ValueStr());
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
						IBK::FormatString("Error converting '"+attrib->ValueStr()+"' attribute (unknown unit).") ), FUNC_ID);
				}
			else if (attribName == "yUnit")
				try {
					m_yUnit = IBK::Unit(attrib->ValueStr());
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
						IBK::FormatString("Error converting '"+attrib->ValueStr()+"' attribute (unknown unit).") ), FUNC_ID);
				}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}
		// search for mandatory elements
		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "IBK:LinearSpline") {
				IBK::LinearSpline p;
				std::string name;
				readLinearSplineElement(c, p, name, nullptr, nullptr);
				bool success = false;
				if (name == "Values") {
					m_values = p; success = true;
				}
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(name).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'LinearSplineParameter' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'LinearSplineParameter' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * LinearSplineParameter::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("LinearSplineParameter");
	parent->LinkEndChild(e);

	if (!m_name.empty())
		e->SetAttribute("name", m_name);
	if (m_interpolationMethod != NUM_I)
		e->SetAttribute("interpolationMethod", KeywordList::Keyword("LinearSplineParameter::interpolationMethod_t",  m_interpolationMethod));
	if (m_wrapMethod != NUM_C)
		e->SetAttribute("wrapMethod", KeywordList::Keyword("LinearSplineParameter::wrapMethod_t",  m_wrapMethod));
	if (m_xUnit.id() != 0)
		e->SetAttribute("xUnit", m_xUnit.name());
	if (m_yUnit.id() != 0)
		e->SetAttribute("yUnit", m_yUnit.name());
	if (!m_values.empty())
		writeLinearSplineElement(e, "Values", m_values, "-", "-");
	return e;
}

} // namespace NANDRAD
