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

#include <tinyxml.h>

namespace NANDRAD {

void DailyCycle::readXML(const TiXmlElement * element) {
	FUNCID("DailyCycle::readXML");

	try {
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


	if (!m_intervals.empty()) {
		TiXmlElement * child = new TiXmlElement("Intervals");
		e->LinkEndChild(child);

		for (std::vector<Interval>::const_iterator ifaceIt = m_intervals.begin();
			ifaceIt != m_intervals.end(); ++ifaceIt)
		{
			ifaceIt->writeXML(child);
		}
	}


	if (!m_hourlyValues.empty()) {
		TiXmlElement * child = new TiXmlElement("HourlyValues");
		e->LinkEndChild(child);

		for (std::vector<LinearSplineParameter>::const_iterator ifaceIt = m_hourlyValues.begin();
			ifaceIt != m_hourlyValues.end(); ++ifaceIt)
		{
			ifaceIt->writeXML(child);
		}
	}

	return e;
}

} // namespace NANDRAD
