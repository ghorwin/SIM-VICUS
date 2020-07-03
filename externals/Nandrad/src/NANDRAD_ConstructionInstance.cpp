/*	The NANDRAD data model library.
Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

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

#include "NANDRAD_ConstructionInstance.h"
#include "NANDRAD_KeywordList.h"

#include <IBK_Parameter.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <IBK_algorithm.h>

#include "NANDRAD_Constants.h"

#include <tinyxml.h>

namespace NANDRAD {

ConstructionInstance::ConstructionInstance() :
	m_constructionTypeId(INVALID_ID)
{
}

void ConstructionInstance::readXML(const TiXmlElement * element) {
	const char * const FUNC_ID = "[ConstructionInstance::readXML]";
	// read attributes

#if 0

	try {
		const TiXmlAttribute * attrib = TiXmlAttribute::attributeByName(element, "displayName");
		if (attrib)
			m_displayName = attrib->Value();

		attrib = TiXmlAttribute::attributeByName(element, "id");

		if (!attrib)
		{
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing 'id' attribute.")
				), FUNC_ID);
		}

		try {
			m_id = IBK::string2val<unsigned int>(attrib->Value());
		}
		// Error obtaining id number
		catch (IBK::Exception & ex) {
			throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Error reading 'id' attribute.")
				), FUNC_ID);
		}

		const TiXmlElement * c;
		// read sub-elements
		for ( c = element->FirstChildElement(); c; c = c->NextSiblingElement()) {
			// determine data based on element name
			std::string cname = c->Value();
			if (cname == "ConstructionTypeID") {
				m_constructionTypeId = IBK::string2val<unsigned int>(c->GetText());
			}
			else if (cname == "IBK:Parameter") {
				// use utility function to read parameter
				std::string namestr, unitstr;
				double value;
				TiXmlElement::readIBKParameterElement(c, namestr, unitstr, value);

				if (KeywordList::KeywordExists("ConstructionInstance::para_t", namestr)) {

					// determine type of parameter
					para_t t = (para_t)KeywordList::Enumeration("ConstructionInstance::para_t", namestr);
					// parameter may be without a unit
					m_para[t].set(namestr, value, unitstr);
					// check parameter unit
					std::string paraUnit = KeywordList::Unit("ConstructionInstance::para_t", t);
					if (unitstr != paraUnit) {
						try {
							m_para[t].get_value(paraUnit);
						}
						catch (IBK::Exception &ex) {
							throw IBK::Exception(ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
								IBK::FormatString("Invalid unit '#%1' of parameter #%2!")
								.arg(paraUnit)
								.arg(namestr)
							), FUNC_ID);
						}
					}
				}
				else {
					readGenericParameterElement(c);
				}
			}
			else if (cname == "EmbeddedObjects")  {
				IBK::read_range_XML(c->FirstChildElement(), m_embeddedObjects);
			}
			else if (cname == "Interfaces") {
				IBK::read_range_XML(c->FirstChildElement(), m_interfaces);
			}
			else if (cname == "FMUExportReference") {
				FMUExportReference exportDef;
				exportDef.readXML(c);
				m_FMUExportReferences.push_back(exportDef);
			}
			else if (cname == "FMUImportReference") {
				FMUImportReference importDef;
				importDef.readXML(c);
				m_FMUImportReferences.push_back(importDef);
			}

			// try to read a generic parametrization
			else {
				try {
					readGenericParameterElement(c);
				} catch (IBK::Exception) {
					throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
						IBK::FormatString("Unknown XML tag with name '%1'.").arg(cname)
						), FUNC_ID);
				}

			}
		}
#define AREA_DEFAULT_HACK
#ifdef AREA_DEFAULT_HACK
		if (m_para[NANDRAD::ConstructionInstance::CP_AREA].name.empty() ) {
			IBK::IBK_Message(IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
				IBK::FormatString("ConstructionInstance [%1] '%2' does not have Area defined, defaulting to 1 m2.")
				.arg(m_id).arg(m_displayName)
				), IBK::MSG_WARNING, FUNC_ID);
			m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", "1 m2");
		}
#endif //
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'ConstructionInstance' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'ConstructionInstance' element.").arg(ex2.what()), FUNC_ID);
	}
#endif
}


void ConstructionInstance::writeXML(TiXmlElement * parent, bool detailedOutput) const {
#if 0
	TiXmlElement * e = new TiXmlElement("ConstructionInstance");
	parent->LinkEndChild(e);

	if (!m_displayName.empty())
		e->SetAttribute("displayName", m_displayName);

	e->SetAttribute("id",IBK::val2string<unsigned int>(m_id));

	// write all construction instance parameters
	for (unsigned int i=0; i<NUM_CP; ++i) {
		if (m_para[i].name.empty()) continue;
		if(detailedOutput)
			TiXmlComment::addComment(e,KeywordList::Description("ConstructionInstance::para_t",i));
		TiXmlElement::appendIBKParameterElement(e, m_para[i].name, m_para[i].IO_unit.name(), m_para[i].get_value());
	}
	// write generic parameters
	writeGenericParameters(e, detailedOutput);

	// write all references
	TiXmlElement::appendSingleAttributeElement(e, "ConstructionTypeID", NULL, std::string(),IBK::val2string<unsigned int>(m_constructionTypeId));

	// now write interfaces
	TiXmlElement * child = new TiXmlElement("Interfaces");
	e->LinkEndChild(child);

	for (std::vector<Interface>::const_iterator ifaceIt = m_interfaces.begin();
		 ifaceIt != m_interfaces.end(); ++ifaceIt)
	{
		if(detailedOutput)
			ifaceIt->writeCommentsXML(child);
		ifaceIt->writeXML(child,detailedOutput);
	}
	// write all embedded objects
	if (!m_embeddedObjects.empty()) {
		TiXmlElement * child = new TiXmlElement("EmbeddedObjects");
		e->LinkEndChild(child);

		for (std::vector<EmbeddedObject>::const_iterator objectIt = m_embeddedObjects.begin();
			 objectIt != m_embeddedObjects.end(); ++objectIt)
		{
			if(detailedOutput)
				objectIt->writeCommentsXML(child);
			objectIt->writeXML(child,detailedOutput);
		}
	}
	// FMU export definitions
	for (unsigned int i = 0; i < m_FMUExportReferences.size(); ++i) {
		// write sensor output
		m_FMUExportReferences[i].writeXML(e);
	}
	// FMU import definitions
	for (unsigned int i = 0; i < m_FMUImportReferences.size(); ++i) {
		// write sensor output
		m_FMUImportReferences[i].writeXML(e);
	}
#endif
}

void ConstructionInstance::writeCommentsXML(TiXmlElement * parent) const {
#if 0
	// first add all comments
	for ( std::set<std::string>::const_iterator it = m_comments.begin();
								it != m_comments.end(); ++it)
	{
		TiXmlComment::addComment(parent,*it);
	}

	// add construction type infomation to comment
	std::string referenceComment = std::string("Wall construction");
	if (m_zoneNames.size() == 2) {
		referenceComment += std::string(" between '") + m_zoneNames[0] + std::string("' and '") + m_zoneNames[1] + std::string("'");
	}
	referenceComment += std::string("'.");
	TiXmlComment::addComment(parent,referenceComment);
#endif
}

#if 0
const EmbeddedObject & ConstructionInstance::embeddedObjectById( const unsigned int id) const {
	const char * const FUNC_ID = "[ConstructionInstance::embeddedObjectById]";

	// preparation for std::map
	std::vector<NANDRAD::EmbeddedObject>::const_iterator embeddedObjectIt =
		m_embeddedObjects.begin();

	for ( ; embeddedObjectIt != m_embeddedObjects.end(); ++embeddedObjectIt)
	{
		if (embeddedObjectIt->m_id == id)
			return *embeddedObjectIt;
	}
	throw IBK::Exception( IBK::FormatString("Requested invalid Embedded object id '%1' inside ConstructionInstance with id %2.")
											.arg(id).arg(m_id),
											FUNC_ID);
}
#endif

bool ConstructionInstance::behavesLike(const ConstructionInstance & other) const {
	if (m_constructionTypeId != other.m_constructionTypeId)
		return false;

	// now compare interface at location A with interface A of other object
	if (m_interfaces.size() != other.m_interfaces.size())
		return false;

	int AIndex = -1;
	int BIndex = -1;
	for (unsigned int i=0; i<m_interfaces.size(); ++i) {
		switch (m_interfaces[i].m_location) {
			case Interface::IT_A : AIndex = i; break;
			case Interface::IT_B : BIndex = i; break;
			default:; // error not necessary, will bail out later
		}
	}
	int AIndexOther = -1;
	int BIndexOther = -1;
	for (unsigned int i=0; i<other.m_interfaces.size(); ++i) {
		switch (other.m_interfaces[i].m_location) {
			case Interface::IT_A : AIndexOther = i; break;
			case Interface::IT_B : BIndexOther = i; break;
			default:; // error not necessary, will bail out later
		}
	}

	bool isOutside = false;
	if (AIndex != -1) {
		if (AIndexOther == -1) return false;
		if (!m_interfaces[AIndex].behavesLike(other.m_interfaces[AIndexOther])) return false;

		if (m_interfaces[AIndex].m_zoneId == 0)
			isOutside = true;
	}
	if (BIndex != -1) {
		if (BIndexOther == -1) return false;
		if (!m_interfaces[BIndex].behavesLike(other.m_interfaces[BIndexOther])) return false;

		if (m_interfaces[BIndex].m_zoneId == 0)
			isOutside = true;
	}

	if (isOutside) {
		if (m_para[CP_ORIENTATION] != other.m_para[CP_ORIENTATION])
			return false;
		if (m_para[CP_INCLINATION] != other.m_para[CP_INCLINATION])
			return false;
	}

	return true; // both construction instances would calculate effectively the same
}

} // namespace NANDRAD

