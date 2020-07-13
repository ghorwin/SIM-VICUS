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

#include <NANDRAD_ConstructionInstance.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>

#include <tinyxml.h>

namespace NANDRAD {

TiXmlElement * ConstructionInstance::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("ConstructionInstance");
	parent->LinkEndChild(e);

	e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_displayName.empty())
		e->SetAttribute("displayName", m_displayName);

	TiXmlElement::appendSingleAttributeElement(e, "ConstructionTypeId", nullptr, std::string(), IBK::val2string<unsigned int>(m_constructionTypeId));

	for (unsigned int i=0; i<NUM_CP; ++i) {
		if (!m_para[i].name.empty())
			TiXmlElement::appendIBKParameterElement(e, m_para[i].name, m_para[i].IO_unit.name(), m_para[i].get_value());
	}

	if (!m_interfaces.empty()) {
		TiXmlElement * child = new TiXmlElement("Interfaces");
		e->LinkEndChild(child);

		for (std::vector<Interface>::const_iterator ifaceIt = m_interfaces.begin();
			ifaceIt != m_interfaces.end(); ++ifaceIt)
		{
			ifaceIt->writeXML(child);
		}
	}


	return e;
}

} // namespace NANDRAD
