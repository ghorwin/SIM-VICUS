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

#include <NANDRAD_Location.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <NANDRAD_Constants.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace NANDRAD {

void Location::readXML(const TiXmlElement * element) {
	FUNCID(Location::readXML);

	try {
		// search for mandatory elements
		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "IBK:Parameter") {
				IBK::Parameter p;
				NANDRAD::readParameterElement(c, p);
				bool success = false;
				para_t ptype;
				try {
					ptype = (para_t)KeywordList::Enumeration("Location::para_t", p.name);
					m_para[ptype] = p; success = true;
				}
				catch (...) { /* intentional fail */  }
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.name).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else if (cName == "IBK:Flag") {
				IBK::Flag f;
				NANDRAD::readFlagElement(c, f);
				bool success = false;
				try {
					flag_t ftype = (flag_t)KeywordList::Enumeration("Location::flag_t", f.name());
					m_flags[ftype] = f; success=true;
				}
				catch (...) { /* intentional fail */  }
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(f.name()).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else if (cName == "TimeZone")
				m_timeZone = NANDRAD::readPODElement<int>(c, cName);
			else if (cName == "ClimateFilePath")
				m_climateFilePath = IBK::Path(c->GetText());
			else if (cName == "ShadingFactorFilePath")
				m_shadingFactorFilePath = IBK::Path(c->GetText());
			else if (cName == "Sensors") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Sensor")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					Sensor obj;
					obj.readXML(c2);
					m_sensors.push_back(obj);
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
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Location' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Location' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * Location::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("Location");
	parent->LinkEndChild(e);


	for (unsigned int i=0; i<NUM_P; ++i) {
		if (!m_para[i].name.empty()) {
			TiXmlElement::appendIBKParameterElement(e, m_para[i].name, m_para[i].IO_unit.name(), m_para[i].get_value(m_para[i].IO_unit));
		}
	}

	for (int i=0; i<NUM_F; ++i) {
		if (!m_flags[i].name().empty()) {
			TiXmlElement::appendSingleAttributeElement(e, "IBK:Flag", "name", m_flags[i].name(), m_flags[i].isEnabled() ? "true" : "false");
		}
	}
	TiXmlElement::appendSingleAttributeElement(e, "TimeZone", nullptr, std::string(), IBK::val2string<int>(m_timeZone));
	if (m_climateFilePath.isValid())
		TiXmlElement::appendSingleAttributeElement(e, "ClimateFilePath", nullptr, std::string(), m_climateFilePath.str());
	if (m_shadingFactorFilePath.isValid())
		TiXmlElement::appendSingleAttributeElement(e, "ShadingFactorFilePath", nullptr, std::string(), m_shadingFactorFilePath.str());

	if (!m_sensors.empty()) {
		TiXmlElement * child = new TiXmlElement("Sensors");
		e->LinkEndChild(child);

		for (std::vector<Sensor>::const_iterator it = m_sensors.begin();
			it != m_sensors.end(); ++it)
		{
			it->writeXML(child);
		}
	}

	return e;
}

} // namespace NANDRAD
