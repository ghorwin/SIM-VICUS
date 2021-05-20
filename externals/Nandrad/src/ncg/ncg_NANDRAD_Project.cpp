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
			if (cName == "Zones") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Zone")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					Zone obj;
					obj.readXML(c2);
					m_zones.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "ConstructionInstances") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "ConstructionInstance")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					ConstructionInstance obj;
					obj.readXML(c2);
					m_constructionInstances.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "HydraulicNetworks") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "HydraulicNetwork")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					HydraulicNetwork obj;
					obj.readXML(c2);
					m_hydraulicNetworks.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "ConstructionTypes") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "ConstructionType")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					ConstructionType obj;
					obj.readXML(c2);
					m_constructionTypes.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Materials") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Material")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					Material obj;
					obj.readXML(c2);
					m_materials.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "WindowGlazingSystems") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "WindowGlazingSystem")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					WindowGlazingSystem obj;
					obj.readXML(c2);
					m_windowGlazingSystems.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "ObjectLists") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "ObjectList")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					ObjectList obj;
					obj.readXML(c2);
					m_objectLists.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "ProjectInfo")
				m_projectInfo.readXML(c);
			else if (cName == "Location")
				m_location.readXML(c);
			else if (cName == "SimulationParameter")
				m_simulationParameter.readXML(c);
			else if (cName == "SolverParameter")
				m_solverParameter.readXML(c);
			else if (cName == "Schedules")
				m_schedules.readXML(c);
			else if (cName == "Models")
				m_models.readXML(c);
			else if (cName == "Outputs")
				m_outputs.readXML(c);
			else if (cName == "FMIDescription")
				m_fmiDescription.readXML(c);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
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

	m_location.writeXML(e);

	m_simulationParameter.writeXML(e);

	m_solverParameter.writeXML(e);

	if (!m_zones.empty()) {
		TiXmlElement * child = new TiXmlElement("Zones");
		e->LinkEndChild(child);

		for (std::vector<Zone>::const_iterator it = m_zones.begin();
			it != m_zones.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_constructionInstances.empty()) {
		TiXmlElement * child = new TiXmlElement("ConstructionInstances");
		e->LinkEndChild(child);

		for (std::vector<ConstructionInstance>::const_iterator it = m_constructionInstances.begin();
			it != m_constructionInstances.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_hydraulicNetworks.empty()) {
		TiXmlElement * child = new TiXmlElement("HydraulicNetworks");
		e->LinkEndChild(child);

		for (std::vector<HydraulicNetwork>::const_iterator it = m_hydraulicNetworks.begin();
			it != m_hydraulicNetworks.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_constructionTypes.empty()) {
		TiXmlElement * child = new TiXmlElement("ConstructionTypes");
		e->LinkEndChild(child);

		for (std::vector<ConstructionType>::const_iterator it = m_constructionTypes.begin();
			it != m_constructionTypes.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_materials.empty()) {
		TiXmlElement * child = new TiXmlElement("Materials");
		e->LinkEndChild(child);

		for (std::vector<Material>::const_iterator it = m_materials.begin();
			it != m_materials.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_windowGlazingSystems.empty()) {
		TiXmlElement * child = new TiXmlElement("WindowGlazingSystems");
		e->LinkEndChild(child);

		for (std::vector<WindowGlazingSystem>::const_iterator it = m_windowGlazingSystems.begin();
			it != m_windowGlazingSystems.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	m_schedules.writeXML(e);

	m_models.writeXML(e);

	m_outputs.writeXML(e);

	if (!m_objectLists.empty()) {
		TiXmlElement * child = new TiXmlElement("ObjectLists");
		e->LinkEndChild(child);

		for (std::vector<ObjectList>::const_iterator it = m_objectLists.begin();
			it != m_objectLists.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	m_fmiDescription.writeXML(e);
	return e;
}

} // namespace NANDRAD
