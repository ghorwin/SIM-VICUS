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

#include <NANDRAD_InterfaceLongWaveEmission.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <NANDRAD_Constants.h>
#include <NANDRAD_KeywordList.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace NANDRAD {

void InterfaceLongWaveEmission::readXMLPrivate(const TiXmlElement * element) {
	FUNCID(InterfaceLongWaveEmission::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "modelType"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'modelType' attribute.") ), FUNC_ID);

		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "modelType")
			try {
				m_modelType = (modelType_t)KeywordList::Enumeration("InterfaceLongWaveEmission::modelType_t", attrib->ValueStr());
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
					IBK::FormatString("Invalid or unknown keyword '"+attrib->ValueStr()+"'.") ), FUNC_ID);
			}
			attrib = attrib->Next();
		}
		// search for mandatory elements
		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "IBK:Parameter") {
				IBK::Parameter p;
				readParameterElement(c, p);
				bool success = false;
				try {
					para_t ptype = (para_t)KeywordList::Enumeration("InterfaceLongWaveEmission::para_t", p.name);
					m_para[ptype] = p;
					std::string refUnit = KeywordList::Unit("InterfaceLongWaveEmission::para_t", ptype);
					if (!refUnit.empty() && (p.IO_unit.base_id() != IBK::Unit(refUnit).base_id())) {
						throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(c->Row())
											  .arg("Incompatible unit '"+p.IO_unit.name()+"', expected '"+refUnit +"'."), FUNC_ID);
					}
					success = true;
				}
				catch (...) { /* intentional fail */  }
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
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'InterfaceLongWaveEmission' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'InterfaceLongWaveEmission' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * InterfaceLongWaveEmission::writeXMLPrivate(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("InterfaceLongWaveEmission");
	parent->LinkEndChild(e);

	if (m_modelType != NUM_MT)
		e->SetAttribute("modelType", KeywordList::Keyword("InterfaceLongWaveEmission::modelType_t",  m_modelType));

	for (unsigned int i=0; i<NUM_P; ++i) {
		if (!m_para[i].name.empty())
			TiXmlElement::appendIBKParameterElement(e, m_para[i].name, m_para[i].IO_unit.name(), m_para[i].get_value());
	}
	return e;
}

} // namespace NANDRAD
