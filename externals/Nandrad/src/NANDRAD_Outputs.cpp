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

#include "NANDRAD_Outputs.h"
#include "NANDRAD_KeywordList.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <IBK_algorithm.h>

#include "NANDRAD_Constants.h"

#include <tinyxml.h>

namespace NANDRAD {


bool Outputs::isDefault( ) const {
	Outputs tmp;
	return (*this == tmp);
}


#if 0
void Outputs::readXML(const TiXmlElement * element) {
	const char * const FUNC_ID = "[Outputs::readXML]";

	try {
		// loop over all elements in this XML element
		for (const TiXmlElement * e = element->FirstChildElement(); e; e = e->NextSiblingElement()) {
			// get element name
			std::string name = e->Value();

			// check for sub-lists

			if (name == "OutputGrids") {
				IBK::read_range_XML(e->FirstChildElement(), m_grids);
			}
			else if (name == "OutputDefinitions") {
				IBK::read_range_XML(e->FirstChildElement(), m_outputDefinitions);
			}
			else if (name == "IBK:Unit") {

				std::string namestr;
				std::string ustr;
				TiXmlElement::readSingleAttributeElement(e, "name", namestr, ustr);

				// currently we only know time unit
				if (namestr != "TimeUnit") {
					throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(e->Row()).arg(
						IBK::FormatString("Unknown unit '%1'.").arg(namestr)
						), FUNC_ID);
				}

				// check if we have a valid unit and if it is a time unit
				IBK::Unit u(ustr);
				if (u.base_id() != IBK::Unit("s").base_id()) {
					throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(e->Row()).arg(
							IBK::FormatString("Unit '%1' is not a time unit.").arg(ustr)
							), FUNC_ID);
				}
				m_timeUnit = u;

			}
			else if (name == "IBK:Flag") {

				// use utility function to read flag
				std::string namestr;
				std::string flag;
				TiXmlElement::readSingleAttributeElement(e, "name", namestr, flag);

				// currently we only know one flag
				if (namestr != "BinaryFormat")
					throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(e->Row()).arg(
						IBK::FormatString("Unknown flag '%1'.").arg(namestr)
						), FUNC_ID);

				m_binaryFormat.set(namestr, (flag == "true" || flag == "1"));
			}
			else
			{
				throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(e->Row()).arg(
					IBK::FormatString("Unknown XML tag '%1'.").arg(name)
					), FUNC_ID);
			}

		} // for
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading 'Outputs' section."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception(IBK::FormatString("%1\nError reading 'Outputs' section.").arg(ex2.what()), FUNC_ID);
	}
}


void Outputs::writeXML(TiXmlElement * parent) const {
	// if all lists are empty, and standard values are set, just return
	if (isDefault())
		return;


	TiXmlElement * e = new TiXmlElement("Outputs");

	// write parameters
	TiXmlComment::addComment(parent, "Contains definitions of output files and schedules for outputs.");
	TiXmlComment::addComment(e, "General parameters");
	// write time unit
	if (!m_timeUnit.name().empty())
			TiXmlElement::appendSingleAttributeElement(e, "IBK:Unit", "name", "TimeUnit",
			m_timeUnit.name());
	// write binary format flag
	if (!m_binaryFormat.name().empty()) {
		TiXmlElement::appendSingleAttributeElement(e, "IBK:Flag", "name", "BinaryFormat",
			m_binaryFormat.isEnabled() ? "true" : "false");
	}

	// write schedules
	TiXmlComment::addComment(parent, "Output specification.");
	parent->LinkEndChild(e);
	TiXmlElement * child1 = new TiXmlElement("OutputGrids");
	e->LinkEndChild(child1);
	IBK::write_range_XML(m_grids.begin(), m_grids.end(), child1);

	// write output files
	TiXmlComment::addComment(e, "List of all Output Definitions");
	TiXmlElement * child2 = new TiXmlElement("OutputDefinitions");
	e->LinkEndChild(child2);
	IBK::write_range_XML(m_outputDefinitions.begin(), m_outputDefinitions.end(), child2);

	TiXmlComment::addSeparatorComment(parent);
}
#endif


bool Outputs::operator==(const Outputs & other) const {
	if (m_outputDefinitions.size() != other.m_outputDefinitions.size()) return false;
	for (unsigned int i=0; i<m_outputDefinitions.size(); ++i)
		if (!(m_outputDefinitions[i] == other.m_outputDefinitions[i])) return false;

	if (m_grids.size() != other.m_grids.size()) return false;
	for (unsigned int i=0; i<m_grids.size(); ++i)
		if (!(m_grids[i] == other.m_grids[i])) return false;

	if (m_binaryFormat != other.m_binaryFormat) return false;
	if (m_timeUnit != other.m_timeUnit) return false;

	return true;
}



} // namespace NANDRAD

