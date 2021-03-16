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

	try {
		// search for mandatory elements
		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "IBK:Parameter") {
				IBK::Parameter p;
				NANDRAD::readParameterElement(c, p);
				bool success = false;
				para_t ptype;
				try {
					ptype = (para_t)KeywordList::Enumeration("NetworkHeatExchange::para_t", p.name);
					m_para[ptype] = p; success = true;
				}
				catch (...) { /* intentional fail */  }
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.name).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else if (cName == "LinearSplineParameter") {
				NANDRAD::LinearSplineParameter p;
				p.readXML(c);
				bool success = false;
				try {
					splinePara_t ptype;
					ptype = (splinePara_t)KeywordList::Enumeration("NetworkHeatExchange::splinePara_t", p.m_name);
					m_splPara[ptype] = p; success = true;
				}
				catch (...) { /* intentional fail */  }
				if (!success)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(p.m_name).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			else if (cName == "ModelType") {
				try {
					m_modelType = (ModelType)KeywordList::Enumeration("NetworkHeatExchange::ModelType", c->GetText());
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
						IBK::FormatString("Invalid or unknown keyword '"+std::string(c->GetText())+"'.") ), FUNC_ID);
				}
			}
			else {
				bool found = false;
				for (int i=0; i<NUM_ID; ++i) {
					if (cName == KeywordList::Keyword("NetworkHeatExchange::References",i)) {
						m_idReferences[i] = (IDType)NANDRAD::readPODElement<unsigned int>(c, cName);
						found = true;
						break;
					}
				}
				if (!found)
					IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'NetworkHeatExchange' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'NetworkHeatExchange' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * NetworkHeatExchange::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("NetworkHeatExchange");
	parent->LinkEndChild(e);


	if (m_modelType != NUM_T)
		TiXmlElement::appendSingleAttributeElement(e, "ModelType", nullptr, std::string(), KeywordList::Keyword("NetworkHeatExchange::ModelType",  m_modelType));

	for (unsigned int i=0; i<NUM_P; ++i) {
		if (!m_para[i].name.empty()) {
			TiXmlElement::appendIBKParameterElement(e, m_para[i].name, m_para[i].IO_unit.name(), m_para[i].get_value());
		}
	}

	for (int i=0; i<NUM_ID; ++i) {
		if (m_idReferences[i] != VICUS::INVALID_ID)
				TiXmlElement::appendSingleAttributeElement(e, KeywordList::Keyword("NetworkHeatExchange::References",  i), nullptr, std::string(), IBK::val2string<unsigned int>(m_idReferences[i]));
	}
	for (int i=0; i<NUM_SPL; ++i) {
		if (!m_splPara[i].m_name.empty()) {
			m_splPara[i].writeXML(e);
		}
	}
	return e;
}

} // namespace VICUS
