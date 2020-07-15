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

#include <NANDRAD_Location.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <NANDRAD_Constants.h>

#include <tinyxml.h>

namespace NANDRAD {

void Location::readXML(const TiXmlElement * element) {
	FUNCID("Location::readXML");

	try {
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


	for (unsigned int i=0; i<NUM_LP; ++i) {
		if (!m_para[i].name.empty())
			TiXmlElement::appendIBKParameterElement(e, m_para[i].name, m_para[i].IO_unit.name(), m_para[i].get_value());
	}
	if (m_climateFileName.isValid())
		TiXmlElement::appendSingleAttributeElement(e, "ClimateFileName", nullptr, std::string(), m_climateFileName.str());
	if (m_shadingFactorFileName.isValid())
		TiXmlElement::appendSingleAttributeElement(e, "ShadingFactorFileName", nullptr, std::string(), m_shadingFactorFileName.str());

	if (!m_sensors.empty()) {
		TiXmlElement * child = new TiXmlElement("Sensors");
		e->LinkEndChild(child);

		for (std::vector<Sensor>::const_iterator ifaceIt = m_sensors.begin();
			ifaceIt != m_sensors.end(); ++ifaceIt)
		{
			ifaceIt->writeXML(child);
		}
	}

	return e;
}

} // namespace NANDRAD
