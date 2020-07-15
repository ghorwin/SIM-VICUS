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

#include "NANDRAD_Location.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_KeywordList.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>

#include <tinyxml.h>

namespace NANDRAD {

Location::Location() :
	m_cyclic(true)
{}


void Location::readXML(const TiXmlElement * element) {
#if 0


	const char * const FUNC_ID = "[Location::readXML]";

	// read parameters
	const TiXmlElement * c;
	try {
		// read sub-elements
		for ( c = element->FirstChildElement(); c; c = c->NextSiblingElement()) {
			// determine data based on element name
			std::string cname = c->Value();
			if (cname == "IBK:Parameter") {
				// use utility function to read parameter
				std::string namestr, unitstr;
				double value;
				TiXmlElement::readIBKParameterElement(c, namestr, unitstr, value);

				if (KeywordList::KeywordExists("Location::para_t",namestr)){

					// determine type of parameter
					para_t t = (para_t)KeywordList::Enumeration("Location::para_t", namestr);
					m_para[t].set( namestr, value, unitstr );

					// check parameter unit
					std::string paraUnit = KeywordList::Unit("Location::para_t", t);
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
				else
				{
					throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
						IBK::FormatString("Unknown IBK:Parameter '%1'.").arg(namestr)
						), FUNC_ID);
				}

			}
			else if (cname == "ClimateReference") {

				const TiXmlAttribute * attrib = TiXmlAttribute::attributeByName(c, "displayName");
				if (attrib)
					m_climateFileDisplayName = attrib->Value();

				const char * const str = c->GetText();
				if (str)	m_climateFileName = str;
				else		m_climateFileName.clear();

				attrib = TiXmlAttribute::attributeByName(c, "cyclic");
				if (attrib) {
					const char * const str = c->GetText();
					if (str == std::string("false"))
						m_cyclic = false;
				}

			}
			else if (cname == "ShadingFactorReference") {

				const TiXmlAttribute * attrib = TiXmlAttribute::attributeByName(c, "displayName");
				if (attrib)
					m_shadingFactorFileDisplayName = attrib->Value();

				const char * const str = c->GetText();
				if (str)	m_shadingFactorFileName = str;
				else		m_shadingFactorFileName.clear();

			}
			else if (cname == "Sensor") {
				Sensor sensor;
				sensor.readXML(c);
				m_sensors.push_back(sensor);
			}
			else if (cname == "FMUExportReference") {
				FMUExportReference exportDef;
				exportDef.readXML(c);
				m_FMUExportReferences.push_back(exportDef);
			}
			else {
				throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
					IBK::FormatString("Unknown XML tag '%1'.").arg(cname)
					), FUNC_ID);
			}
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading 'Location' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception(IBK::FormatString("%1\nError reading 'Location' element.").arg(ex2.what()), FUNC_ID);
	}
#endif
}


void Location::writeXML(TiXmlElement * parent) const {
#if 0

	TiXmlComment::addComment(parent,
		"Location of the building.");
	TiXmlElement * e = new TiXmlElement("Location");
	parent->LinkEndChild(e);

	// write parameters
	for(unsigned int i = 0; i < NUM_LP; ++i) {
		if(m_para[i].name.empty()) continue;
		if(detailedOutput)
			TiXmlComment::addComment(e,KeywordList::Description("Location::para_t",i));
		TiXmlElement::appendIBKParameterElement(e,
			m_para[i].name, m_para[i].IO_unit.name(), m_para[i].get_value());
	}
	// climate reference
	if ( !m_climateFileName.str().empty() ) {

		if(detailedOutput)
			TiXmlComment::addComment(e,"Reference to climate data set.");

		TiXmlElement * climateRef = new TiXmlElement("ClimateReference");
		e->LinkEndChild( climateRef );

		if(!m_climateFileDisplayName.empty())
			climateRef->SetAttribute("displayName", m_climateFileDisplayName);
		TiXmlText * text = new TiXmlText( m_climateFileName.str() );
		climateRef->LinkEndChild( text );

	}
	// shading reference
	if (!m_shadingFactorFileName.str().empty()) {

		if (detailedOutput)
			TiXmlComment::addComment(e, "Reference to shading factor file.");

		TiXmlElement * shadingRef = new TiXmlElement("ShadingFactorReference");
		e->LinkEndChild(shadingRef);

		if (!m_shadingFactorFileDisplayName.empty())
			shadingRef->SetAttribute("displayName", m_shadingFactorFileDisplayName);
		TiXmlText * text = new TiXmlText(m_shadingFactorFileName.str());
		shadingRef->LinkEndChild(text);
	}
	// sensors
	for (unsigned int i = 0; i < m_sensors.size(); ++i) {
		// write sensor output
		m_sensors[i].writeXML(e);
	}
	// FMU export definitions
	for (unsigned int i = 0; i < m_FMUExportReferences.size(); ++i) {
		// write sensor output
		m_FMUExportReferences[i].writeXML(e);
	}
	TiXmlComment::addSeparatorComment(parent);
#endif
}

} // namespace NANDRAD

