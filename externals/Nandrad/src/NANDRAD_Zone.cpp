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

#include "NANDRAD_Zone.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_KeywordList.h"
#include "NANDRAD_SpaceType.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>

#include <tinyxml.h>

namespace NANDRAD {

Zone::Zone() :
	m_id(NANDRAD::INVALID_ID),
	m_spaceType("Default"),
	m_zoneType(NUM_ZT),
	m_location(NUM_ZL),
	m_spaceTypeRef(nullptr)
{
}

void Zone::readXML(const TiXmlElement * element) {
	const char * const FUNC_ID = "[Zone::readXML]";
#if 0
	try {
		// read attributes
		const TiXmlAttribute * attrib = TiXmlAttribute::attributeByName(element, "displayName");
		if (attrib)
			m_displayName = attrib->Value();
		attrib = TiXmlAttribute::attributeByName(element, "id");
		if (!attrib)
			throw IBK::Exception(IBK::FormatString("Expected 'id' attribute in Zone. Please check line: %1 .").arg( element->Row() ), FUNC_ID);
		m_id = IBK::string2val<unsigned int>(attrib->Value());

		// read temperature type
		attrib = TiXmlAttribute::attributeByName(element, "type");
		if (!attrib)
		{
			throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing 'type' attribute.")
				), FUNC_ID);
		}

		if(!KeywordList::KeywordExists("Zone::zoneType_t",attrib->Value()) )
		{
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Unknown Zone type '%1'.").arg(attrib->Value())
				), FUNC_ID);
		}

		m_zoneType = (zoneType_t) KeywordList::Enumeration("Zone::zoneType_t",attrib->Value());

		// an active or detailed zone is always declared as inside zone
		if (m_zoneType == ZT_ACTIVE || m_zoneType == ZT_DETAILED) {
			m_location = ZL_INSIDE;
		}
		// for constant zone check location attribute
		else if (m_zoneType == ZT_CONSTANT) {
			// read temperature type
			attrib = TiXmlAttribute::attributeByName(element, "location");
			if (attrib)
			{
				if (!KeywordList::KeywordExists("Zone::location_t", attrib->Value()))
				{
					throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
						IBK::FormatString("Unknown location '%1'.").arg(attrib->Value())
					), FUNC_ID);
				}

				m_location = (location_t)KeywordList::Enumeration("Zone::location_t", attrib->Value());
			}
		}
		// for a ground zone set attribute
		else if (m_zoneType == ZT_GROUND) {
			m_location = ZL_GROUND;
		}

		// read parameters
		const TiXmlElement * c;
		// read sub-elements
		for ( c = element->FirstChildElement(); c; c = c->NextSiblingElement()) {
			// determine data based on element name
			std::string cname = c->Value();
			if (cname == "SpaceTypeName") {
				m_spaceType = c->GetText();
			}
			else if (cname == "IBK:Parameter") {
				// use utility function to read parameter
				std::string namestr, unitstr;
				double value;
				TiXmlElement::readIBKParameterElement(c, namestr, unitstr, value);
				// check if we have a predefined parameter
				if(KeywordList::KeywordExists("Zone::para_t", namestr) )
				{
					// determine type of parameter
					para_t t = (para_t)KeywordList::Enumeration("Zone::para_t", namestr);
					m_para[t].set(namestr, value, unitstr);
				}
				// we have a generic parameter
				else
				{
					readGenericParameterElement(c);
				}
			}
			// try to integer parameters
			else if (KeywordList::KeywordExists("Zone::intpara_t",cname) ) {
				intpara_t p = (intpara_t)KeywordList::Enumeration("Zone::intpara_t", cname);
				int val;
				std::stringstream strm(c->GetText());
				if (strm >> val)
					m_intpara[p].set(cname, val);
				else {
					throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
						IBK::FormatString("Invalid parameter value for '%1' property.").arg(cname)
						), FUNC_ID);
				}
			}
			// try to read view factors
			else if (cname == "ViewFactors") {
				std::string viewFactorString;
				std::stringstream str(c->GetText());

				// get first line
				while(std::getline(str,viewFactorString,'\n')) {
					IBK::trim(viewFactorString);
					if(viewFactorString.empty())
						continue;
					std::vector<std::string> tokens;
					IBK::explode(viewFactorString, tokens, ' ', true);
					// alterantively search for tab string
					if(tokens.size() != 3)
						IBK::explode(viewFactorString, tokens, '\t', true);
					if (tokens.size() != 3)
						throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
							IBK::FormatString("Wrong ViewFactor format!")
							),FUNC_ID);

					viewFactorPair idPair(IBK::string2val<unsigned int>(tokens[0]),
										IBK::string2val<unsigned int>(tokens[1]));
					m_viewFactors.push_back(std::make_pair
						(idPair, IBK::string2val<double>(tokens[2])) );
				}
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
			// try to read climate reference
			else if (cname == "ClimateReference") {

				const TiXmlAttribute * attrib = TiXmlAttribute::attributeByName(c, "displayName");
				if (attrib)
					m_climateFileDisplayName = attrib->Value();

				const char * const str = c->GetText();
				if (str)	m_climateFileName = str;
				else m_climateFileName.clear();
			}
			// try to read a generic parametrization
			else {
				readGenericParameterElement(c);
			}
		}
		// error: constant zone without location
		if (m_zoneType == ZT_CONSTANT && m_location == NUM_ZL) {
			throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Attribute 'location' is required for zones with type 'Constant'.")
				), FUNC_ID);
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading 'Zone' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception(IBK::FormatString("%1\nError reading 'Zone' element.").arg(ex2.what()), FUNC_ID);
	}
#endif
}

void Zone::writeXML(TiXmlElement * parent, bool detailedOutput) const {
#if 0
	// first add all comments
	for ( std::set<std::string>::const_iterator it = m_comments.begin();
								it != m_comments.end(); ++it)
		TiXmlComment::addComment(parent,*it);

	TiXmlElement * e = new TiXmlElement("Zone");
	parent->LinkEndChild(e);

	if (!m_displayName.empty())
		e->SetAttribute("displayName", m_displayName);

	e->SetAttribute("id", m_id);
	e->SetAttribute("type", KeywordList::Keyword("Zone::zoneType_t", m_zoneType));

	// constant zones always have a location
	if (m_zoneType == ZT_CONSTANT)
		e->SetAttribute("location", KeywordList::Keyword("Zone::location_t", m_location));

	if (m_spaceType != std::string("Default") )
		TiXmlElement::appendSingleAttributeElement(e,"SpaceTypeName", nullptr, std::string(), m_spaceType);

	// write zone parameters
	for (unsigned int i=0; i<NUM_ZP; ++i) {
		if(m_para[i].name.empty()) continue;
		if(detailedOutput)
			TiXmlComment::addComment(e,KeywordList::Description("Zone::para_t",i));
		TiXmlElement::appendIBKParameterElement(e,
			m_para[i].name,
			m_para[i].IO_unit.name(),
			m_para[i].get_value());
	}
	// write view factors
	if(!m_viewFactors.empty() ) {
		std::string str("\n");
		for(unsigned int i = 0; i < m_viewFactors.size(); ++i) {
			str += IBK::val2string<unsigned int>(m_viewFactors[i].first.first) + std::string(" ") +
					IBK::val2string<unsigned int>(m_viewFactors[i].first.second) + std::string(" ") +
					IBK::val2string<double>(m_viewFactors[i].second) + std::string("\n");
		}
		TiXmlElement::appendSingleAttributeElement(e,"ViewFactors", nullptr, std::string(), str);
	}
	// write climate reference file
	if (!m_climateFileDisplayName.empty() && !m_climateFileName.str().empty()) {
		TiXmlElement::appendSingleAttributeElement(e, "ClimateReference", "displayName", m_climateFileDisplayName,
			m_climateFileName.c_str());
	}
	else if (!m_climateFileName.str().empty()) {
		TiXmlElement::appendSingleAttributeElement(e, "ClimateReference", nullptr, std::string(),
			m_climateFileName.c_str());
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
	// write generic parameters
	writeGenericParameters(e, detailedOutput);
#endif
}

} // namespace NANDRAD

