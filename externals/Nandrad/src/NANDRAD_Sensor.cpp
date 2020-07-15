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

#include <IBK_Parameter.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>

#include <tinyxml.h>

namespace NANDRAD {

void Sensor::readXMLPrivate(const TiXmlElement * element) {
	FUNCID(Sensor::readXMLPrivate);

	try {
		// read attributes
		const TiXmlAttribute * attrib = TiXmlAttribute::attributeByName(element, "id");
		if (attrib != nullptr) {
			throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing 'id' attribute.") ), FUNC_ID);
		}
		// convert attribute to value
		try {
			m_id = IBK::string2val<unsigned int>(attrib->ValueStr());
		} catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Invalid value for 'id' attribute.") ), FUNC_ID);
		}

		// read parameters
		for (const TiXmlElement * c = element->FirstChildElement(); c; c = c->NextSiblingElement()) {
			// determine data based on element name
			std::string cname = c->Value();
			if (cname == "Quantity") {
				m_quantity = c->GetText();
				if (m_quantity.empty())
					throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
						"Tag 'Quantity' must not be empty."), FUNC_ID);
			}
			else {
				throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
					IBK::FormatString("Undefined tag '%1'.").arg(cname) ), FUNC_ID);
			}
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading 'Sensor' element."), FUNC_ID);
	}
	catch (std::exception & ex) {
		throw IBK::Exception(IBK::FormatString("%1\nError reading 'Sensor' element.").arg(ex.what()), FUNC_ID);
	}
}


TiXmlElement * Sensor::writeXMLPrivate(TiXmlElement * parent, bool /*detailedOutput*/) const {
	TiXmlElement * e = new TiXmlElement("Sensor");
	parent->LinkEndChild(e);

	e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));

	if (!m_quantity.empty())
		TiXmlElement::appendSingleAttributeElement(e, "Quantity", nullptr, std::string(), m_quantity);

	return e;
}


void Sensor::readXML(const TiXmlElement * element) {
	// simply reuse generated code
	readXMLPrivate(element);

	// ... read other data from element
}


TiXmlElement * Sensor::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = writeXMLPrivate(parent);

	// .... append other data to e
	return e;
}



bool Sensor::operator!=(const Sensor & other) const {
	if (m_id != other.m_id)				return true;
	if (m_quantity != other.m_quantity)	return true;
	return false;
}

} // namespace NANDRAD

