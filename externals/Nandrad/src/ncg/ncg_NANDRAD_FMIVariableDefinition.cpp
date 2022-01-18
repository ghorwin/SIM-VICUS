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

#include <NANDRAD_FMIVariableDefinition.h>
#include <NANDRAD_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <NANDRAD_Constants.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace NANDRAD {

void FMIVariableDefinition::readXML(const TiXmlElement * element) {
	FUNCID(FMIVariableDefinition::readXML);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "fmiVarName"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'fmiVarName' attribute.") ), FUNC_ID);

		if (!TiXmlAttribute::attributeByName(element, "unit"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'unit' attribute.") ), FUNC_ID);

		if (!TiXmlAttribute::attributeByName(element, "fmiValueRef"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'fmiValueRef' attribute.") ), FUNC_ID);

		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "fmiVarName")
				m_fmiVarName = attrib->ValueStr();
			else if (attribName == "unit")
				m_unit = attrib->ValueStr();
			else if (attribName == "fmiValueRef")
				m_fmiValueRef = (IDType)NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}
		// search for mandatory elements
		if (!element->FirstChildElement("FmiStartValue"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'FmiStartValue' element.") ), FUNC_ID);

		if (!element->FirstChildElement("VarName"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'VarName' element.") ), FUNC_ID);

		if (!element->FirstChildElement("ObjectId"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'ObjectId' element.") ), FUNC_ID);

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "FmiVarDescription")
				m_fmiVarDescription = c->GetText();
			else if (cName == "FmiTypeName")
				m_fmiTypeName = c->GetText();
			else if (cName == "FmiStartValue")
				m_fmiStartValue = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "VarName")
				m_varName = c->GetText();
			else if (cName == "ObjectId")
				m_objectId = (IDType)NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "VectorIndex")
				m_vectorIndex = (IDType)NANDRAD::readPODElement<unsigned int>(c, cName);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'FMIVariableDefinition' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'FMIVariableDefinition' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * FMIVariableDefinition::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("FMIVariableDefinition");
	parent->LinkEndChild(e);

	if (!m_fmiVarName.empty())
		e->SetAttribute("fmiVarName", m_fmiVarName);
	if (!m_unit.empty())
		e->SetAttribute("unit", m_unit);
	if (m_fmiValueRef != NANDRAD::INVALID_ID)
		e->SetAttribute("fmiValueRef", IBK::val2string<IDType>(m_fmiValueRef));
	if (!m_fmiVarDescription.empty())
		TiXmlElement::appendSingleAttributeElement(e, "FmiVarDescription", nullptr, std::string(), m_fmiVarDescription);
	if (!m_fmiTypeName.empty())
		TiXmlElement::appendSingleAttributeElement(e, "FmiTypeName", nullptr, std::string(), m_fmiTypeName);
	TiXmlElement::appendSingleAttributeElement(e, "FmiStartValue", nullptr, std::string(), IBK::val2string<double>(m_fmiStartValue));
	if (!m_varName.empty())
		TiXmlElement::appendSingleAttributeElement(e, "VarName", nullptr, std::string(), m_varName);
	if (m_objectId != NANDRAD::INVALID_ID)
			TiXmlElement::appendSingleAttributeElement(e, "ObjectId", nullptr, std::string(), IBK::val2string<unsigned int>(m_objectId));
	if (m_vectorIndex != NANDRAD::INVALID_ID)
			TiXmlElement::appendSingleAttributeElement(e, "VectorIndex", nullptr, std::string(), IBK::val2string<unsigned int>(m_vectorIndex));
	return e;
}

} // namespace NANDRAD
