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

#include <NANDRAD_SimulationParameter.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <NANDRAD_Constants.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace NANDRAD {

void SimulationParameter::readXML(const TiXmlElement * element) {
	FUNCID("SimulationParameter::readXML");

	try {
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "IBK:Parameter") {
				IBK::Parameter p;
				readParameterElement(c, cName, p);
				if (p.name == "Para[NUM_SP]") {
				}
				else {
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.name).arg(cName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
				}
			}
			else if (cName == "IBK:IntPara") {
				IBK::Parameter p;
				readParameterElement(c, cName, p);
				if (p.name == "Intpara[NUM_SIP]") {
				}
				else {
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.name).arg(cName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
				}
			}
			else if (cName == "StringPara[NUM_SSP]")
				m_stringPara[NUM_SSP] = c->GetText();
			else if (cName == "IBK:Flag") {
				IBK::Flag f;
				readFlagElement(c, cName, f);
				if (f.name() == "Flags[NUM_SF]") {
				}
				else {
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(f.name()).arg(cName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
				}
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'SimulationParameter' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'SimulationParameter' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * SimulationParameter::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("SimulationParameter");
	parent->LinkEndChild(e);


	for (unsigned int i=0; i<NUM_SP; ++i) {
		if (!m_para[i].name.empty())
			TiXmlElement::appendIBKParameterElement(e, m_para[i].name, m_para[i].IO_unit.name(), m_para[i].get_value());
	}

	for (unsigned int i=0; i<NUM_SIP; ++i) {
		if (!m_intpara[i].name.empty())
			TiXmlElement::appendIBKParameterElement(e, m_intpara[i].name, std::string(), m_intpara[i].value, true);
	}

	TiXmlElement::appendSingleAttributeElement(e, "StringPara[NUM_SSP]", nullptr, std::string(), m_stringPara[NUM_SSP]);

	for (int i=0; i<NUM_SF; ++i) {
		if (!m_flags[i].name().empty())
			TiXmlElement::appendSingleAttributeElement(e, "IBK:Flag", "name", m_flags[i].name(), m_flags[i].isEnabled() ? "true" : "false");
	}

	m_interval.writeXML(e);
	return e;
}

} // namespace NANDRAD
