/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "VICUS_EpdCategoryDataset.h"

#include <IBK_MessageHandler.h>
#include <IBK_messages.h>

#include <VICUS_KeywordList.h>.h>

#include <NANDRAD_Utilities.h>

#include <tinyxml.h>


namespace VICUS {

EpdCategoryDataset::~EpdCategoryDataset() {

}

EpdCategoryDataset EpdCategoryDataset::scaleByFactor(const double & factor) const {
	EpdCategoryDataset epd = *this;
	for(unsigned int i=0; i<NUM_P; ++i)
		epd.m_para[i].value = factor * m_para[i].value; // no testing needed so straight forward
	return epd;
}

void EpdCategoryDataset::readXML(const TiXmlElement * element) {
	FUNCID(EpdCategoryDataset::readXML);

	try {
		// search for mandatory elements
		if (!element->FirstChildElement("std::vector<Module>"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'std::vector<Module>' element.") ), FUNC_ID);

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "Modules") {
				const TiXmlElement * c2 = c->FirstChildElement();
				while (c2) {
					const std::string & c2Name = c2->ValueStr();
					if (c2Name != "Module")
						IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);

					std::string str = c2->ValueStr();
					std::vector<std::string> strings = IBK::explode(str, ',');
					try {
						for(const std::string &s : strings) {
							EpdCategoryDataset::Module module = static_cast<EpdCategoryDataset::Module>(VICUS::KeywordList::Enumeration("EpdCategoryDataset::Module", s));
							m_modules.push_back(module);
						}
					}
					catch(IBK::Exception &ex) {
						throw IBK::Exception( ex, IBK::FormatString("Error reading 'EpdCategoryDataset' Modules '%1'.").arg(str), FUNC_ID);
					}

					c2 = c2->NextSiblingElement();
				}
			}
			else if (cName == "IBK:Parameter") {
				IBK::Parameter p;
				NANDRAD::readParameterElement(c, p);
				bool success = false;
				para_t ptype;
				try {
					ptype = (para_t)KeywordList::Enumeration("EpdCategoryDataset::para_t", p.name);
					m_para[ptype] = p; success = true;
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
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'EpdCategoryDataset' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'EpdCategoryDataset' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * EpdCategoryDataset::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("EpdCategoryDataset");
	parent->LinkEndChild(e);


	if (!m_modules.empty()) {
		TiXmlElement * child = new TiXmlElement("Modules");
		e->LinkEndChild(child);

		std::string moduleString;
		int count = 0;
		for (std::vector<Module>::const_iterator it = m_modules.begin();
			it != m_modules.end(); ++it)
		{
			moduleString += std::string(count > 0 ? ", " : "") + VICUS::KeywordList::Description("EpdCategoryDataset::Module", *it); // add ", " in between beginning from the second module
			++count;
		}
	}


	for (unsigned int i=0; i<NUM_P; ++i) {
		if (!m_para[i].name.empty()) {
			TiXmlElement::appendIBKParameterElement(e, m_para[i].name, m_para[i].IO_unit.name(), m_para[i].get_value(m_para[i].IO_unit));
		}
	}
	return e;

}

}
