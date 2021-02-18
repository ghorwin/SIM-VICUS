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

#include <VICUS_DailyCycle.h>
#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <VICUS_Constants.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace VICUS {

void DailyCycle::readXML(const TiXmlElement * element) {
	FUNCID(DailyCycle::readXML);

	try {
		// search for mandatory elements
		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "DayTypes")
				NANDRAD::readVector(c, "DayTypes", m_dayTypes);
			else if (cName == "TimePoints")
				NANDRAD::readVector(c, "TimePoints", m_timePoints);
			else if (cName == "Values")
				NANDRAD::readVector(c, "Values", m_values);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
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

	NANDRAD::writeVector(e, "DayTypes", m_dayTypes);
	NANDRAD::writeVector(e, "TimePoints", m_timePoints);
	NANDRAD::writeVector(e, "Values", m_values);
	return e;
}

} // namespace VICUS
