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

#include "NANDRAD_EmbeddedObject.h"
#include "NANDRAD_KeywordList.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <IBK_messages.h>

#include <tinyxml.h>

#include "NANDRAD_Constants.h"

namespace NANDRAD {

EmbeddedObject::EmbeddedObject() :
	m_id(INVALID_ID)
{
}

EmbeddedObject::objectType_t EmbeddedObject::objectType() const {

#if 0
	if ( m_window.m_modelType != EmbeddedObjectWindow::NUM_MT )
		return OT_WINDOW;

	if ( m_door.m_modelType != EmbeddedObjectDoor::NUM_MT )
		return OT_DOOR;

	if ( m_hole.m_modelType != EmbeddedObjectHole::NUM_MT )
		return OT_HOLE;

#define MISSING_OT_GIVES_WARNING
#ifdef MISSING_OT_GIVES_WARNING
	IBK::IBK_Message(
		IBK::FormatString("Missing model parametrization for Embedded Object with id %1.").arg(m_id), IBK::MSG_WARNING, "[EmbeddedObject::objectType]");
	return NUM_OT;
#else // MISSING_OT_GIVES_WARNING
	throw IBK::Exception(
		IBK::FormatString("Missing model parametrization for Embedded Object with id %1. Please check your embedded objects in project file.").arg(m_id),
				"[EmbeddedObject::objectType]"
				);
#endif

#endif
	return EmbeddedObject::NUM_OT;
}


#if 0
void EmbeddedObject::readXML(const TiXmlElement * element) {
	const char * const FUNC_ID = "[EmbeddedObject::readXML]";

	try {

		// read attributes
		const TiXmlAttribute * attrib = TiXmlAttribute::attributeByName(element, "displayName");
		if (attrib)
			m_displayName = attrib->Value();

		attrib = TiXmlAttribute::attributeByName(element, "id");
		if (!attrib)
			throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing 'id' attribute.")
				), FUNC_ID);

		try {
			m_id = IBK::string2val<unsigned int>(attrib->Value());
			NANDRAD::IDGeneratorSingleton::instance().setNextFreeId( NANDRAD::IDGeneratorSingleton::IDS_Interface, m_id+1 );
		}
		// Error obtaining id number
		catch (IBK::Exception & ex) {
			throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Error reading 'id' attribute.")
				), FUNC_ID);
		}

		// read parameters
		// read sub-elements
		for (const TiXmlElement * c = element->FirstChildElement(); c; c = c->NextSiblingElement()) {

			// determine data based on element name
			std::string cname = c->Value();
			if (cname == "IBK:Parameter") {

				// use utility function to read parameter
				std::string namestr, unitstr;
				double value;
				TiXmlElement::readIBKParameterElement(c, namestr, unitstr, value);

				// determine type of parameter
				para_t t = (para_t)KeywordList::Enumeration("EmbeddedObject::para_t", namestr);
				m_para[t].set(namestr, value, unitstr);

				// check parameter unit
				std::string paraUnit = KeywordList::Unit("EmbeddedObject::para_t", t);
				if (unitstr != paraUnit) {
					try {
						m_para[t].get_value(paraUnit);
					}
					catch (IBK::Exception &ex) {
						throw IBK::Exception(ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
							IBK::FormatString("Invalid unit '#%1' of parameter #%2!")
							.arg(paraUnit).arg(namestr)
							), FUNC_ID);
					}
				}
			}
			else if ( cname == KeywordList::Keyword("EmbeddedObject::objectType_t", OT_WINDOW)) {

				/// \todo STVO asks: This is done to prevent more then one embedded object in one wall???
				if ( m_objectWasRead )
					throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(c->Row())
						.arg(IBK::FormatString("Multiple model parameter sections.") )
						, FUNC_ID);

				// read window properties
				m_window.readXML(c);
				m_objectWasRead = true;

			}
			else if ( cname == KeywordList::Keyword("EmbeddedObject::objectType_t", OT_DOOR)) {

				/// \todo STVO asks: This is done to prevent more then one embedded object in one wall???
				if ( m_objectWasRead )
					throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(c->Row())
						.arg(IBK::FormatString("Multiple model parameter sections.") )
						, FUNC_ID);

				// read door model properties
				m_door.readXML(c);
				m_objectWasRead = true;

			} else if ( cname == KeywordList::Keyword("EmbeddedObject::objectType_t", OT_HOLE)) {

				/// \todo STVO asks: This is done to prevent more then one embedded object in one wall???
				if ( m_objectWasRead )
					throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(c->Row())
						.arg(IBK::FormatString("Multiple model parameter sections.") )
						, FUNC_ID);

				// hole has no properties yet
				m_hole.readXML(c);
				m_objectWasRead = true;

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
			else {
				throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(c->Row())
						.arg(IBK::FormatString("Unknown XML tag with name '%1'.").arg(cname)
						), FUNC_ID);
			}
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading constant 'EmbeddedObject' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading constant 'EmbeddedObject' element.").arg(ex2.what()), FUNC_ID);
	}
}


void EmbeddedObject::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("EmbeddedObject");
	parent->LinkEndChild(e);

	if(!m_displayName.empty())
		e->SetAttribute("displayName", m_displayName);

	e->SetAttribute( "id", IBK::val2string<unsigned int>( m_id ) );

	// write EmbeddedObject parameters
	if (detailedOutput)
		TiXmlComment::addComment(e,std::string("Constant object parameters"));

	for (unsigned int i=0; i<NUM_P; ++i) {
		if(m_para[i].name.empty()) continue;
		if(detailedOutput)
			TiXmlComment::addComment(e,KeywordList::Description("EmbeddedObject::para_t",i));
		TiXmlElement::appendIBKParameterElement(e,
			m_para[i].name,
			m_para[i].IO_unit.name(),
			m_para[i].get_value());
	}

	// write model parameter section
	switch ( objectType() ) {
		case OT_WINDOW :
			m_window.writeXML(e);
			break;
		case OT_DOOR :
			m_door.writeXML(e);
			break;
		case OT_HOLE :
			m_hole.writeXML(e);
			break;

		default : ;
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
}
#endif


} // namespace NANDRAD

