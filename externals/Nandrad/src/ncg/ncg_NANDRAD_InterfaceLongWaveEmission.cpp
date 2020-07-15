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

#include <NANDRAD_InterfaceLongWaveEmission.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <NANDRAD_Constants.h>
#include <NANDRAD_KeywordList.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace NANDRAD {

void InterfaceLongWaveEmission::readXML(const TiXmlElement * element) {
	FUNCID("InterfaceLongWaveEmission::readXML");

	try {
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			else if (cName == "IBK:Parameter") {
				IBK::Parameter p;
				readParameterElement(c, cName, p);
				if (p.name == "Para[NUM_P]") {
				}
				else {
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.name).arg(cName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
				}
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'InterfaceLongWaveEmission' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'InterfaceLongWaveEmission' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * InterfaceLongWaveEmission::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("InterfaceLongWaveEmission");
	parent->LinkEndChild(e);


	if (m_modelType != NUM_MT)
		TiXmlElement::appendSingleAttributeElement(e, "ModelType", nullptr, std::string(), KeywordList::Keyword("InterfaceLongWaveEmission::modelType_t",  m_modelType));

	m_modelTypeToParameterMapping.writeXML(e);

	for (unsigned int i=0; i<NUM_P; ++i) {
		if (!m_para[i].name.empty())
			TiXmlElement::appendIBKParameterElement(e, m_para[i].name, m_para[i].IO_unit.name(), m_para[i].get_value());
	}
	return e;
}

} // namespace NANDRAD
