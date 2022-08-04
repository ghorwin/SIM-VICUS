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

#include "VICUS_PlainGeometry.h"

#include <tinyxml.h>

namespace VICUS {

void PlainGeometry::readXML(const TiXmlElement * element) {
	FUNCID(PlainGeometry::readXML);
	// read 3D geometry
	VICUS::Surface surf;
	const TiXmlElement * c = element->FirstChildElement();
	while (c) {
		const std::string & cName = c->ValueStr();
		// needed in order to read old flat hierarchy
		// <PlainGeometry>
		//		<Surface/>
		// </PlainGeometry>
		if (cName == "Surface") {
			try {
				surf.readXML(c);
			} catch (IBK::Exception & ex) {
				throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
					IBK::FormatString("Error reading Surface tag.") ), FUNC_ID);
			}
			// copy surface to plain geometry
			m_surfaces.push_back(surf);
			// next surface
			c = c->NextSiblingElement();
		}
		else
			break;
	}

	readXMLPrivate(element);
}

TiXmlElement * PlainGeometry::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = writeXMLPrivate(parent);
	// now add Polygon3D
	// m_geometry.polygon3D().writeXML(e);
	return e;
}

} // namespace VICUS
