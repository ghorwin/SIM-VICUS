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

#include <NANDRAD_HydraulicNetworkSoilModel.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <NANDRAD_Constants.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace NANDRAD {

void HydraulicNetworkSoilModel::readXML(const TiXmlElement * element) {
	FUNCID(HydraulicNetworkSoilModel::readXML);

	try {
		// search for mandatory attributes
		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = (IDType)NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}
		// search for mandatory elements
		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "SupplyPipeIds")
				NANDRAD::readVector(c, "SupplyPipeIds", m_supplyPipeIds);
			else if (cName == "ReturnPipeIds")
				NANDRAD::readVector(c, "ReturnPipeIds", m_returnPipeIds);
			else if (cName == "IBK:Parameter") {
				IBK::Parameter p;
				NANDRAD::readParameterElement(c, p);
				bool success = false;
				if (p.name == "PipeSpacing") {
					m_pipeSpacing = p; success = true;
				}
				else if (p.name == "PipeDepth") {
					m_pipeDepth = p; success = true;
				}
				else if (p.name == "PipeOuterDiameter") {
					m_pipeOuterDiameter = p; success = true;
				}
				if (!success) {
				}
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.name).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'HydraulicNetworkSoilModel' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'HydraulicNetworkSoilModel' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * HydraulicNetworkSoilModel::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("HydraulicNetworkSoilModel");
	parent->LinkEndChild(e);

	e->SetAttribute("id", IBK::val2string<IDType>(m_id));
	NANDRAD::writeVector(e, "SupplyPipeIds", m_supplyPipeIds);
	NANDRAD::writeVector(e, "ReturnPipeIds", m_returnPipeIds);
	if (!m_pipeSpacing.name.empty()) {
		IBK_ASSERT("PipeSpacing" == m_pipeSpacing.name);
		TiXmlElement::appendIBKParameterElement(e, "PipeSpacing", m_pipeSpacing.IO_unit.name(), m_pipeSpacing.get_value());
	}
	if (!m_pipeDepth.name.empty()) {
		IBK_ASSERT("PipeDepth" == m_pipeDepth.name);
		TiXmlElement::appendIBKParameterElement(e, "PipeDepth", m_pipeDepth.IO_unit.name(), m_pipeDepth.get_value());
	}
	if (!m_pipeOuterDiameter.name.empty()) {
		IBK_ASSERT("PipeOuterDiameter" == m_pipeOuterDiameter.name);
		TiXmlElement::appendIBKParameterElement(e, "PipeOuterDiameter", m_pipeOuterDiameter.IO_unit.name(), m_pipeOuterDiameter.get_value());
	}
	return e;
}

} // namespace NANDRAD
