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

#include <NANDRAD_Outputs.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <NANDRAD_Constants.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace NANDRAD {

void Outputs::readXML(const TiXmlElement * element) {
	FUNCID("Outputs::readXML");

	try {
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			else if (cName == "TimeUnit")
				m_timeUnit = readUnitElement(c, cName);
			else if (cName == "IBK:Flag") {
				IBK::Flag f;
				readFlagElement(c, cName, f);
				if (f.name() == "BinaryFormat") {
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
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Outputs' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Outputs' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * Outputs::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("Outputs");
	parent->LinkEndChild(e);


	if (!m_outputDefinitions.empty()) {
		TiXmlElement * child = new TiXmlElement("OutputDefinitions");
		e->LinkEndChild(child);

		for (std::vector<OutputDefinition>::const_iterator ifaceIt = m_outputDefinitions.begin();
			ifaceIt != m_outputDefinitions.end(); ++ifaceIt)
		{
			ifaceIt->writeXML(child);
		}
	}


	if (!m_grids.empty()) {
		TiXmlElement * child = new TiXmlElement("Grids");
		e->LinkEndChild(child);

		for (std::vector<OutputGrid >::const_iterator ifaceIt = m_grids.begin();
			ifaceIt != m_grids.end(); ++ifaceIt)
		{
			ifaceIt->writeXML(child);
		}
	}


	TiXmlElement::appendSingleAttributeElement(e, "TimeUnit", nullptr, std::string(), m_timeUnit.name());

	TiXmlElement::appendSingleAttributeElement(e, "IBK:Flag", "name", m_binaryFormat.name(), m_binaryFormat.isEnabled() ? "true" : "false");
	return e;
}

} // namespace NANDRAD
