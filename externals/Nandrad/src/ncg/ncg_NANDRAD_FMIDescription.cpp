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

#include <NANDRAD_FMIDescription.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <NANDRAD_Constants.h>

#include <tinyxml.h>

namespace NANDRAD {

void FMIDescription::readXML(const TiXmlElement * element) {
	FUNCID(FMIDescription::readXML);

	try {
		// search for mandatory elements
		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "ModelName")
				m_modelName = c->GetText();
			else if (cName == "InputVariables") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "FMIVariableDefinition")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					FMIVariableDefinition obj;
					obj.readXML(c2);
					m_inputVariables.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "OutputVariables") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "FMIVariableDefinition")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					FMIVariableDefinition obj;
					obj.readXML(c2);
					m_outputVariables.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'FMIDescription' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'FMIDescription' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * FMIDescription::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("FMIDescription");
	parent->LinkEndChild(e);

	if (!m_modelName.empty())
		TiXmlElement::appendSingleAttributeElement(e, "ModelName", nullptr, std::string(), m_modelName);

	if (!m_inputVariables.empty()) {
		TiXmlElement * child = new TiXmlElement("InputVariables");
		e->LinkEndChild(child);

		for (std::vector<FMIVariableDefinition>::const_iterator it = m_inputVariables.begin();
			it != m_inputVariables.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_outputVariables.empty()) {
		TiXmlElement * child = new TiXmlElement("OutputVariables");
		e->LinkEndChild(child);

		for (std::vector<FMIVariableDefinition>::const_iterator it = m_outputVariables.begin();
			it != m_outputVariables.end(); ++it)
		{
			it->writeXML(child);
		}
	}

	return e;
}

} // namespace NANDRAD
