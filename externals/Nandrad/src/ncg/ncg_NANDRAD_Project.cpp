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

#include <NANDRAD_Project.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <NANDRAD_Constants.h>

#include <tinyxml.h>

namespace NANDRAD {

void Project::readXMLPrivate(const TiXmlElement * element) {
	FUNCID(Project::readXMLPrivate);

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
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Project' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Project' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * Project::writeXMLPrivate(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("Project");
	parent->LinkEndChild(e);


	m_projectInfo.writeXML(e);

	if (!m_zones.empty()) {
		TiXmlElement * child = new TiXmlElement("Zones");
		e->LinkEndChild(child);

		for (std::vector<Zone>::const_iterator ifaceIt = m_zones.begin();
			ifaceIt != m_zones.end(); ++ifaceIt)
		{
			ifaceIt->writeXML(child);
		}
	}


	if (!m_constructionInstances.empty()) {
		TiXmlElement * child = new TiXmlElement("ConstructionInstances");
		e->LinkEndChild(child);

		for (std::vector<ConstructionInstance>::const_iterator ifaceIt = m_constructionInstances.begin();
			ifaceIt != m_constructionInstances.end(); ++ifaceIt)
		{
			ifaceIt->writeXML(child);
		}
	}


	if (!m_constructionTypes.empty()) {
		TiXmlElement * child = new TiXmlElement("ConstructionTypes");
		e->LinkEndChild(child);

		for (std::vector<ConstructionType>::const_iterator ifaceIt = m_constructionTypes.begin();
			ifaceIt != m_constructionTypes.end(); ++ifaceIt)
		{
			ifaceIt->writeXML(child);
		}
	}


	if (!m_materials.empty()) {
		TiXmlElement * child = new TiXmlElement("Materials");
		e->LinkEndChild(child);

		for (std::vector<Material>::const_iterator ifaceIt = m_materials.begin();
			ifaceIt != m_materials.end(); ++ifaceIt)
		{
			ifaceIt->writeXML(child);
		}
	}


	m_simulationParameter.writeXML(e);

	m_solverParameter.writeXML(e);

	m_location.writeXML(e);

	m_schedules.writeXML(e);

	m_outputs.writeXML(e);

	if (!m_objectLists.empty()) {
		TiXmlElement * child = new TiXmlElement("ObjectLists");
		e->LinkEndChild(child);

		for (std::vector<ObjectList>::const_iterator ifaceIt = m_objectLists.begin();
			ifaceIt != m_objectLists.end(); ++ifaceIt)
		{
			ifaceIt->writeXML(child);
		}
	}

	return e;
}

} // namespace NANDRAD
