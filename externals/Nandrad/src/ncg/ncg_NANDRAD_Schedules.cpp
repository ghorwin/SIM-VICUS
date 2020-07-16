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

#include <NANDRAD_Schedules.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <NANDRAD_Constants.h>

#include <tinyxml.h>

namespace NANDRAD {

void Schedules::readXMLPrivate(const TiXmlElement * element) {
	FUNCID(Schedules::readXMLPrivate);

	try {
		// search for mandatory elements
		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Schedules' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Schedules' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * Schedules::writeXMLPrivate(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("Schedules");
	parent->LinkEndChild(e);


	if (!m_scheduleGroups.empty()) {
		TiXmlElement * child = new TiXmlElement("ScheduleGroups");
		e->LinkEndChild(child);

		for (std::vector<ScheduleGroup>::const_iterator ifaceIt = m_scheduleGroups.begin();
			ifaceIt != m_scheduleGroups.end(); ++ifaceIt)
		{
			ifaceIt->writeXML(child);
		}
	}


	m_annualSchedules.writeXML(e);
	return e;
}

} // namespace NANDRAD
