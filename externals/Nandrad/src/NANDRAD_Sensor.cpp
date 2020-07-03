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

#include "NANDRAD_Sensor.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_KeywordList.h"
#include "NANDRAD_ObjectList.h"

#include <IBK_Parameter.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>

#include <tinyxml.h>

namespace NANDRAD {

Sensor::Sensor() :
	m_id(NANDRAD::INVALID_ID)
{
}

void Sensor::readXML(const TiXmlElement * element) {
#if 0
	const char * const FUNC_ID = "[Sensor::readXML]";

	try {
		// read attributes
		const TiXmlAttribute * attrib = TiXmlAttribute::attributeByName(element, "id");
		if (!attrib)
		{
			throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing 'id' attribute.")
				), FUNC_ID);
		}

		try {
			m_id = IBK::string2val<unsigned int>(attrib->Value());
			NANDRAD::IDGeneratorSingleton::instance().setNextFreeId( NANDRAD::IDGeneratorSingleton::IDS_Sensor, m_id+1 );
		}
		// Error obtaining id number
		catch (IBK::Exception & ex) {
			throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Error reading 'id' attribute.")
				), FUNC_ID);
		}

		// read parameters
		for (const TiXmlElement * c = element->FirstChildElement(); c; c = c->NextSiblingElement()) {
			// determine data based on element name
			std::string cname = c->Value();
			if (cname == "Quantity") {
				m_quantity = c->GetText();
			}
			else {
				// every remaining tag interpret as a generic parameter block
				readGenericParameterElement(c);
			}
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading 'Sensor' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception(IBK::FormatString("%1\nError reading 'Sensor' element.").arg(ex2.what()), FUNC_ID);
	}
#endif
}

void Sensor::writeXML(TiXmlElement * parent, bool detailedOutput) const {
#if 0
	TiXmlElement * e = new TiXmlElement("Sensor");
	parent->LinkEndChild(e);

	e->SetAttribute("id",IBK::val2string<unsigned int>(m_id));

	if (!m_quantity.empty())
		TiXmlElement::appendSingleAttributeElement(e, "Quantity", NULL, std::string(), m_quantity);

	// write all parameters
	writeGenericParameters(e,detailedOutput);
#endif
}

} // namespace NANDRAD

