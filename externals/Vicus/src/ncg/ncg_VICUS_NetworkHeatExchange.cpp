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

#include <VICUS_NetworkHeatExchange.h>
#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <VICUS_Constants.h>
#include <NANDRAD_Utilities.h>
#include <VICUS_KeywordList.h>

#include <tinyxml.h>

namespace VICUS {

void NetworkHeatExchange::readXML(const TiXmlElement * element) {
	FUNCID(NetworkHeatExchange::readXML);

//	try {
//		// search for mandatory elements
//		// reading elements
//		const TiXmlElement * c = element->FirstChildElement();
//		while (c) {
//			const std::string & cName = c->ValueStr();
//			if (cName == "IBK:Parameter") {
//				IBK::Parameter p;
//				NANDRAD::readParameterElement(c, p);
//				bool success = false;
//				para_t ptype;
//				try {
//					ptype = (para_t)KeywordList::Enumeration("NetworkHeatExchange::Parameter", p.name);
//					m_para[ptype] = p; success = true;
//				}
//				catch (...) { /* intentional fail */  }
//				if (!success)
//					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.name).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
//			}
//			else if (cName == "IBK:IntPara") {
//				IBK::IntPara p;
//				NANDRAD::readIntParaElement(c, p);
//				bool success = false;
//				try {
//					IntParameter ptype = (IntParameter)KeywordList::Enumeration("NetworkHeatExchange::IntParameter", p.name);
//					m_idReferences[ptype] = p; success = true;
//				}
//				catch (...) { /* intentional fail */  }
//				if (!success)
//					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.name).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
//			}
//			else if (cName == "LinearSplineParameter") {
//				NANDRAD::LinearSplineParameter p;
//				p.readXML(c);
//				bool success = false;
//				if (p.m_name == "HeatExchangeSpline") {
//					m_heatExchangeSpline = p; success = true;
//				}
//				if (!success)
//					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.m_name).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
//			}
//			else if (cName == "ModelType") {
//				try {
//					m_modelType = (ModelType)KeywordList::Enumeration("NetworkHeatExchange::ModelType", c->GetText());
//				}
//				catch (IBK::Exception & ex) {
//					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
//						IBK::FormatString("Invalid or unknown keyword '"+std::string(c->GetText())+"'.") ), FUNC_ID);
//				}
//			}
//			else {
//				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
//			}
//			c = c->NextSiblingElement();
//		}
//	}
//	catch (IBK::Exception & ex) {
//		throw IBK::Exception( ex, IBK::FormatString("Error reading 'NetworkHeatExchange' element."), FUNC_ID);
//	}
//	catch (std::exception & ex2) {
//		throw IBK::Exception( IBK::FormatString("%1\nError reading 'NetworkHeatExchange' element.").arg(ex2.what()), FUNC_ID);
//	}
}

TiXmlElement * NetworkHeatExchange::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("NetworkHeatExchange");
	parent->LinkEndChild(e);


//	if (m_modelType != NUM_HT)
//		TiXmlElement::appendSingleAttributeElement(e, "ModelType", nullptr, std::string(), KeywordList::Keyword("NetworkHeatExchange::ModelType",  m_modelType));

//	for (unsigned int i=0; i<NUM_P; ++i) {
//		if (!m_para[i].name.empty()) {
//			TiXmlElement::appendIBKParameterElement(e, m_para[i].name, m_para[i].IO_unit.name(), m_para[i].get_value());
//		}
//	}

//	for (unsigned int i=0; i<NUM_IP; ++i) {
//		if (!m_idReferences[i].name.empty()) {
//			TiXmlElement::appendSingleAttributeElement(e, "IBK:IntPara", "name", m_idReferences[i].name, IBK::val2string(m_idReferences[i].value));
//		}
//	}
//	if (!m_heatExchangeSpline.m_name.empty()) {
//		IBK_ASSERT("HeatExchangeSpline" == m_heatExchangeSpline.m_name);
//		m_heatExchangeSpline.writeXML(e);
//	}
	return e;
}

} // namespace VICUS
