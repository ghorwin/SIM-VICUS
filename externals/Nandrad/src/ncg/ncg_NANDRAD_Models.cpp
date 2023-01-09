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

#include <NANDRAD_Models.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <NANDRAD_Constants.h>

#include <tinyxml.h>

namespace NANDRAD {

void Models::readXML(const TiXmlElement * element) {
	FUNCID(Models::readXML);

	try {
		// search for mandatory elements
		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "NaturalVentilationModels") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "NaturalVentilationModel")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					NaturalVentilationModel obj;
					obj.readXML(c2);
					m_naturalVentilationModels.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "InternalLoadsModels") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "InternalLoadsModel")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					InternalLoadsModel obj;
					obj.readXML(c2);
					m_internalLoadsModels.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "InternalMoistureLoadsModels") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "InternalMoistureLoadsModel")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					InternalMoistureLoadsModel obj;
					obj.readXML(c2);
					m_internalMoistureLoadsModels.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "ShadingControlModels") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "ShadingControlModel")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					ShadingControlModel obj;
					obj.readXML(c2);
					m_shadingControlModels.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "Thermostats") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Thermostat")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					Thermostat obj;
					obj.readXML(c2);
					m_thermostats.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "IdealHeatingCoolingModels") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "IdealHeatingCoolingModel")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					IdealHeatingCoolingModel obj;
					obj.readXML(c2);
					m_idealHeatingCoolingModels.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "IdealSurfaceHeatingCoolingModels") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "IdealSurfaceHeatingCoolingModel")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					IdealSurfaceHeatingCoolingModel obj;
					obj.readXML(c2);
					m_idealSurfaceHeatingCoolingModels.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "IdealPipeRegisterModels") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "IdealPipeRegisterModel")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					IdealPipeRegisterModel obj;
					obj.readXML(c2);
					m_idealPipeRegisterModels.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "HeatLoadSummationModels") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "HeatLoadSummationModel")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					HeatLoadSummationModel obj;
					obj.readXML(c2);
					m_heatLoadSummationModels.push_back(obj);
					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "NetworkInterfaceAdapterModels") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "NetworkInterfaceAdapterModel")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
					NetworkInterfaceAdapterModel obj;
					obj.readXML(c2);
					m_networkInterfaceAdapterModels.push_back(obj);
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
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Models' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Models' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * Models::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("Models");
	parent->LinkEndChild(e);


	if (!m_naturalVentilationModels.empty()) {
		TiXmlElement * child = new TiXmlElement("NaturalVentilationModels");
		e->LinkEndChild(child);

		for (std::vector<NaturalVentilationModel>::const_iterator it = m_naturalVentilationModels.begin();
			it != m_naturalVentilationModels.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_internalLoadsModels.empty()) {
		TiXmlElement * child = new TiXmlElement("InternalLoadsModels");
		e->LinkEndChild(child);

		for (std::vector<InternalLoadsModel>::const_iterator it = m_internalLoadsModels.begin();
			it != m_internalLoadsModels.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_internalMoistureLoadsModels.empty()) {
		TiXmlElement * child = new TiXmlElement("InternalMoistureLoadsModels");
		e->LinkEndChild(child);

		for (std::vector<InternalMoistureLoadsModel>::const_iterator it = m_internalMoistureLoadsModels.begin();
			it != m_internalMoistureLoadsModels.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_shadingControlModels.empty()) {
		TiXmlElement * child = new TiXmlElement("ShadingControlModels");
		e->LinkEndChild(child);

		for (std::vector<ShadingControlModel>::const_iterator it = m_shadingControlModels.begin();
			it != m_shadingControlModels.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_thermostats.empty()) {
		TiXmlElement * child = new TiXmlElement("Thermostats");
		e->LinkEndChild(child);

		for (std::vector<Thermostat>::const_iterator it = m_thermostats.begin();
			it != m_thermostats.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_idealHeatingCoolingModels.empty()) {
		TiXmlElement * child = new TiXmlElement("IdealHeatingCoolingModels");
		e->LinkEndChild(child);

		for (std::vector<IdealHeatingCoolingModel>::const_iterator it = m_idealHeatingCoolingModels.begin();
			it != m_idealHeatingCoolingModels.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_idealSurfaceHeatingCoolingModels.empty()) {
		TiXmlElement * child = new TiXmlElement("IdealSurfaceHeatingCoolingModels");
		e->LinkEndChild(child);

		for (std::vector<IdealSurfaceHeatingCoolingModel>::const_iterator it = m_idealSurfaceHeatingCoolingModels.begin();
			it != m_idealSurfaceHeatingCoolingModels.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_idealPipeRegisterModels.empty()) {
		TiXmlElement * child = new TiXmlElement("IdealPipeRegisterModels");
		e->LinkEndChild(child);

		for (std::vector<IdealPipeRegisterModel>::const_iterator it = m_idealPipeRegisterModels.begin();
			it != m_idealPipeRegisterModels.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_heatLoadSummationModels.empty()) {
		TiXmlElement * child = new TiXmlElement("HeatLoadSummationModels");
		e->LinkEndChild(child);

		for (std::vector<HeatLoadSummationModel>::const_iterator it = m_heatLoadSummationModels.begin();
			it != m_heatLoadSummationModels.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_networkInterfaceAdapterModels.empty()) {
		TiXmlElement * child = new TiXmlElement("NetworkInterfaceAdapterModels");
		e->LinkEndChild(child);

		for (std::vector<NetworkInterfaceAdapterModel>::const_iterator it = m_networkInterfaceAdapterModels.begin();
			it != m_networkInterfaceAdapterModels.end(); ++it)
		{
			it->writeXML(child);
		}
	}

	return e;
}

} // namespace NANDRAD
