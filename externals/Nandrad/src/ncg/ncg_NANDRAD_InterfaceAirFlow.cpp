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

#include <NANDRAD_InterfaceAirFlow.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <NANDRAD_Constants.h>
#include <NANDRAD_KeywordList.h>

#include <tinyxml.h>

namespace NANDRAD {

void InterfaceAirFlow::readXML(const TiXmlElement * element) {
	FUNCID("InterfaceAirFlow::readXML");

	try {
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'InterfaceAirFlow' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'InterfaceAirFlow' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * InterfaceAirFlow::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("InterfaceAirFlow");
	parent->LinkEndChild(e);


	if (m_modelType != NUM_MT)
		TiXmlElement::appendSingleAttributeElement(e, "ModelType", nullptr, std::string(), KeywordList::Keyword("InterfaceAirFlow::modelType_t",  m_modelType));

	m_modelTypeToSplineParameterMapping.writeXML(e);

	m_splinePara[NUM_SP].writeXML(e);
	return e;
}

} // namespace NANDRAD
