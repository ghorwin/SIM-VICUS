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

#include "NANDRAD_SpaceType.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_KeywordList.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>

#include <tinyxml.h>

namespace NANDRAD {


void SpaceType::readXML(const TiXmlElement * element) {
#if 0
	const char * const FUNC_ID = "[SpaceType::readXML]";

	// clear all default quantities
	m_genericParaString.clear();

	try {
		// read attributes
		const TiXmlAttribute * attrib = TiXmlAttribute::attributeByName(element, "name");
		if (!attrib)
		{
			throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing 'name' attribute.")
				), FUNC_ID);
		}
		m_name = attrib->ValueStr();
		// read all parameters
		// read sub-elements
		for (const TiXmlElement * c = element->FirstChildElement(); c; c = c->NextSiblingElement()) {
			// determine data based on element name
			std::string cname = c->Value();
			// read generic parameters
			readGenericParameterElement(c);
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading 'SpaceType' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception(IBK::FormatString("%1\nError reading 'SpaceType' element.").arg(ex2.what()), FUNC_ID);
	}
#endif
}


void SpaceType::writeXML(TiXmlElement * parent) const {
#if 0
	/*SpaceType tmp;
	// tmp.initDefaults();
	if (*this == tmp)
		return;*/

	if ( isDefault() )
		return;

	TiXmlElement * e = new TiXmlElement("SpaceType");
	parent->LinkEndChild(e);

	e->SetAttribute("name", m_name);

	// write all generic parameters
	writeGenericParameters(e);
#endif
}


bool SpaceType::operator!=(const SpaceType & other) const {
#if 0
	if( GenericParametrizationObject (*this) != GenericParametrizationObject (other) )
		return true;

	if (m_name != other.m_name)
		return true;
#endif
	return false;
}


} // namespace NANDRAD

