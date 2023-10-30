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

#include <VICUS_EmbeddedDatabase.h>
#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <VICUS_Constants.h>

#include <tinyxml.h>

namespace VICUS {

void EmbeddedDatabase::readXML(const TiXmlElement * element) {
	FUNCID(EmbeddedDatabase::readXML);

	try {
		// search for mandatory elements
		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "Materials") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Material")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					VICUS::Material obj;
					obj.readXML(c2);
					m_materials.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Constructions") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Construction")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					VICUS::Construction obj;
					obj.readXML(c2);
					m_constructions.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Windows") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Window")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					VICUS::Window obj;
					obj.readXML(c2);
					m_windows.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "WindowGlazingSystems") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "WindowGlazingSystem")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					VICUS::WindowGlazingSystem obj;
					obj.readXML(c2);
					m_windowGlazingSystems.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "AcousticBoundaryConditions") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "AcousticBoundaryCondition")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					VICUS::AcousticBoundaryCondition obj;
					obj.readXML(c2);
					m_acousticBoundaryConditions.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "AcousticSoundAbsorptions") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "AcousticSoundAbsorption")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					VICUS::AcousticSoundAbsorption obj;
					obj.readXML(c2);
					m_acousticSoundAbsorptions.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "BoundaryConditions") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "BoundaryCondition")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					VICUS::BoundaryCondition obj;
					obj.readXML(c2);
					m_boundaryConditions.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Components") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Component")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					VICUS::Component obj;
					obj.readXML(c2);
					m_components.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "SubSurfaceComponents") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "SubSurfaceComponent")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					VICUS::SubSurfaceComponent obj;
					obj.readXML(c2);
					m_subSurfaceComponents.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "SurfaceHeatings") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "SurfaceHeating")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					VICUS::SurfaceHeating obj;
					obj.readXML(c2);
					m_surfaceHeatings.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "SupplySystems") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "SupplySystem")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					VICUS::SupplySystem obj;
					obj.readXML(c2);
					m_supplySystems.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Pipes") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "NetworkPipe")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					VICUS::NetworkPipe obj;
					obj.readXML(c2);
					m_pipes.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Fluids") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "NetworkFluid")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					VICUS::NetworkFluid obj;
					obj.readXML(c2);
					m_fluids.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "NetworkComponents") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "NetworkComponent")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					VICUS::NetworkComponent obj;
					obj.readXML(c2);
					m_networkComponents.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "NetworkControllers") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "NetworkController")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					VICUS::NetworkController obj;
					obj.readXML(c2);
					m_networkControllers.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "SubNetworks") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "SubNetwork")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					VICUS::SubNetwork obj;
					obj.readXML(c2);
					m_subNetworks.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "EPDDatasets") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "EpdDataset")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					VICUS::EpdDataset obj;
					obj.readXML(c2);
					m_EPDDatasets.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Schedules") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Schedule")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					VICUS::Schedule obj;
					obj.readXML(c2);
					m_schedules.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "InternalLoads") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "InternalLoad")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					VICUS::InternalLoad obj;
					obj.readXML(c2);
					m_internalLoads.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "ZoneControlThermostats") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "ZoneControlThermostat")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					VICUS::ZoneControlThermostat obj;
					obj.readXML(c2);
					m_zoneControlThermostats.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "ZoneControlShading") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "ZoneControlShading")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					VICUS::ZoneControlShading obj;
					obj.readXML(c2);
					m_zoneControlShading.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "ZoneIdealHeatingCooling") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "ZoneIdealHeatingCooling")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					VICUS::ZoneIdealHeatingCooling obj;
					obj.readXML(c2);
					m_zoneIdealHeatingCooling.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "ZoneControlVentilationNatural") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "ZoneControlNaturalVentilation")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					VICUS::ZoneControlNaturalVentilation obj;
					obj.readXML(c2);
					m_zoneControlVentilationNatural.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "VentilationNatural") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "VentilationNatural")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					VICUS::VentilationNatural obj;
					obj.readXML(c2);
					m_ventilationNatural.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Infiltration") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Infiltration")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					VICUS::Infiltration obj;
					obj.readXML(c2);
					m_infiltration.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "ZoneTemplates") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "ZoneTemplate")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					VICUS::ZoneTemplate obj;
					obj.readXML(c2);
					m_zoneTemplates.push_back(obj);
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
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'EmbeddedDatabase' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'EmbeddedDatabase' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * EmbeddedDatabase::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("EmbeddedDatabase");
	parent->LinkEndChild(e);


	if (!m_materials.empty()) {
		TiXmlElement * child = new TiXmlElement("Materials");
		e->LinkEndChild(child);

		for (std::vector<VICUS::Material>::const_iterator it = m_materials.begin();
			it != m_materials.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_constructions.empty()) {
		TiXmlElement * child = new TiXmlElement("Constructions");
		e->LinkEndChild(child);

		for (std::vector<VICUS::Construction>::const_iterator it = m_constructions.begin();
			it != m_constructions.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_windows.empty()) {
		TiXmlElement * child = new TiXmlElement("Windows");
		e->LinkEndChild(child);

		for (std::vector<VICUS::Window>::const_iterator it = m_windows.begin();
			it != m_windows.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_windowGlazingSystems.empty()) {
		TiXmlElement * child = new TiXmlElement("WindowGlazingSystems");
		e->LinkEndChild(child);

		for (std::vector<VICUS::WindowGlazingSystem>::const_iterator it = m_windowGlazingSystems.begin();
			it != m_windowGlazingSystems.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_acousticBoundaryConditions.empty()) {
		TiXmlElement * child = new TiXmlElement("AcousticBoundaryConditions");
		e->LinkEndChild(child);

		for (std::vector<VICUS::AcousticBoundaryCondition>::const_iterator it = m_acousticBoundaryConditions.begin();
			it != m_acousticBoundaryConditions.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_acousticSoundAbsorptions.empty()) {
		TiXmlElement * child = new TiXmlElement("AcousticSoundAbsorptions");
		e->LinkEndChild(child);

		for (std::vector<VICUS::AcousticSoundAbsorption>::const_iterator it = m_acousticSoundAbsorptions.begin();
			it != m_acousticSoundAbsorptions.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_boundaryConditions.empty()) {
		TiXmlElement * child = new TiXmlElement("BoundaryConditions");
		e->LinkEndChild(child);

		for (std::vector<VICUS::BoundaryCondition>::const_iterator it = m_boundaryConditions.begin();
			it != m_boundaryConditions.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_components.empty()) {
		TiXmlElement * child = new TiXmlElement("Components");
		e->LinkEndChild(child);

		for (std::vector<VICUS::Component>::const_iterator it = m_components.begin();
			it != m_components.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_subSurfaceComponents.empty()) {
		TiXmlElement * child = new TiXmlElement("SubSurfaceComponents");
		e->LinkEndChild(child);

		for (std::vector<VICUS::SubSurfaceComponent>::const_iterator it = m_subSurfaceComponents.begin();
			it != m_subSurfaceComponents.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_surfaceHeatings.empty()) {
		TiXmlElement * child = new TiXmlElement("SurfaceHeatings");
		e->LinkEndChild(child);

		for (std::vector<VICUS::SurfaceHeating>::const_iterator it = m_surfaceHeatings.begin();
			it != m_surfaceHeatings.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_supplySystems.empty()) {
		TiXmlElement * child = new TiXmlElement("SupplySystems");
		e->LinkEndChild(child);

		for (std::vector<VICUS::SupplySystem>::const_iterator it = m_supplySystems.begin();
			it != m_supplySystems.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_pipes.empty()) {
		TiXmlElement * child = new TiXmlElement("Pipes");
		e->LinkEndChild(child);

		for (std::vector<VICUS::NetworkPipe>::const_iterator it = m_pipes.begin();
			it != m_pipes.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_fluids.empty()) {
		TiXmlElement * child = new TiXmlElement("Fluids");
		e->LinkEndChild(child);

		for (std::vector<VICUS::NetworkFluid>::const_iterator it = m_fluids.begin();
			it != m_fluids.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_networkComponents.empty()) {
		TiXmlElement * child = new TiXmlElement("NetworkComponents");
		e->LinkEndChild(child);

		for (std::vector<VICUS::NetworkComponent>::const_iterator it = m_networkComponents.begin();
			it != m_networkComponents.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_networkControllers.empty()) {
		TiXmlElement * child = new TiXmlElement("NetworkControllers");
		e->LinkEndChild(child);

		for (std::vector<VICUS::NetworkController>::const_iterator it = m_networkControllers.begin();
			it != m_networkControllers.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_subNetworks.empty()) {
		TiXmlElement * child = new TiXmlElement("SubNetworks");
		e->LinkEndChild(child);

		for (std::vector<VICUS::SubNetwork>::const_iterator it = m_subNetworks.begin();
			it != m_subNetworks.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_EPDDatasets.empty()) {
		TiXmlElement * child = new TiXmlElement("EPDDatasets");
		e->LinkEndChild(child);

		for (std::vector<VICUS::EpdDataset>::const_iterator it = m_EPDDatasets.begin();
			it != m_EPDDatasets.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_schedules.empty()) {
		TiXmlElement * child = new TiXmlElement("Schedules");
		e->LinkEndChild(child);

		for (std::vector<VICUS::Schedule>::const_iterator it = m_schedules.begin();
			it != m_schedules.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_internalLoads.empty()) {
		TiXmlElement * child = new TiXmlElement("InternalLoads");
		e->LinkEndChild(child);

		for (std::vector<VICUS::InternalLoad>::const_iterator it = m_internalLoads.begin();
			it != m_internalLoads.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_zoneControlThermostats.empty()) {
		TiXmlElement * child = new TiXmlElement("ZoneControlThermostats");
		e->LinkEndChild(child);

		for (std::vector<VICUS::ZoneControlThermostat>::const_iterator it = m_zoneControlThermostats.begin();
			it != m_zoneControlThermostats.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_zoneControlShading.empty()) {
		TiXmlElement * child = new TiXmlElement("ZoneControlShading");
		e->LinkEndChild(child);

		for (std::vector<VICUS::ZoneControlShading>::const_iterator it = m_zoneControlShading.begin();
			it != m_zoneControlShading.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_zoneIdealHeatingCooling.empty()) {
		TiXmlElement * child = new TiXmlElement("ZoneIdealHeatingCooling");
		e->LinkEndChild(child);

		for (std::vector<VICUS::ZoneIdealHeatingCooling>::const_iterator it = m_zoneIdealHeatingCooling.begin();
			it != m_zoneIdealHeatingCooling.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_zoneControlVentilationNatural.empty()) {
		TiXmlElement * child = new TiXmlElement("ZoneControlVentilationNatural");
		e->LinkEndChild(child);

		for (std::vector<VICUS::ZoneControlNaturalVentilation>::const_iterator it = m_zoneControlVentilationNatural.begin();
			it != m_zoneControlVentilationNatural.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_ventilationNatural.empty()) {
		TiXmlElement * child = new TiXmlElement("VentilationNatural");
		e->LinkEndChild(child);

		for (std::vector<VICUS::VentilationNatural>::const_iterator it = m_ventilationNatural.begin();
			it != m_ventilationNatural.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_infiltration.empty()) {
		TiXmlElement * child = new TiXmlElement("Infiltration");
		e->LinkEndChild(child);

		for (std::vector<VICUS::Infiltration>::const_iterator it = m_infiltration.begin();
			it != m_infiltration.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_zoneTemplates.empty()) {
		TiXmlElement * child = new TiXmlElement("ZoneTemplates");
		e->LinkEndChild(child);

		for (std::vector<VICUS::ZoneTemplate>::const_iterator it = m_zoneTemplates.begin();
			it != m_zoneTemplates.end(); ++it)
		{
			it->writeXML(child);
		}
	}

	return e;
}

} // namespace VICUS
