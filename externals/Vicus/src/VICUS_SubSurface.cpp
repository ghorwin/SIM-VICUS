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

#include "VICUS_SubSurface.h"
#include <tinyxml.h>

namespace VICUS {

void SubSurface::updateColor() {
	if (m_subSurfaceComponentInstance == nullptr) {
		// no subsurface assigned -> opaque light gray
		m_color = QColor(164,164,164,255);
	}
	else {
		// TODO : depending on assigned subsurface type, select suitable color
		// for now we only have windows, and these are always transparent blue
		m_color = QColor(96,96,255,64);
	}
}

void SubSurface::readXML(const TiXmlElement * element) {
	FUNCID(SubSurface::readXML);
	const TiXmlElement * c = element->FirstChildElement();
	while (c) {
		const std::string & cName = c->ValueStr();
		if(cName == "ViewFactors") {
			const TiXmlElement * cc = c->FirstChildElement();
			while (cc) {
				const std::string & ccName = cc->ValueStr();
				if(ccName == "ViewFactor"){
					const TiXmlAttribute * attrib = TiXmlAttribute::attributeByName(cc, "id");
					if(attrib == nullptr){
						throw IBK::Exception("Error reading ViewFactor tag. Has no id attribute!", FUNC_ID);
					}
					unsigned int id = IBK::string2val<unsigned int>(attrib->ValueStr());
					double viewFactor = IBK::string2val<double>(cc->FirstChild()->ValueStr());
					m_viewFactors[id] = viewFactor;
				} else {
					throw IBK::Exception("Error reading ViewFactors tag. Should only contain ViewFactor Tags", FUNC_ID);
				}
				cc = cc->NextSiblingElement();
			}
			// remove ViewFactor element from parent, to avoid getting spammed with "unknown ViewFactor" warning
			const_cast<TiXmlElement *>(element)->RemoveChild(const_cast<TiXmlElement *>(c));
		}
		c = c->NextSiblingElement();
	}
	readXMLPrivate(element);
}

TiXmlElement * SubSurface::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = writeXMLPrivate(parent);
	if(m_viewFactors.size() > 0){
		TiXmlElement * viewFactors = new TiXmlElement("ViewFactors");
		for(const std::pair<unsigned int, double> entry : m_viewFactors){
			TiXmlElement * viewFactor = new TiXmlElement("ViewFactor");
			viewFactor->SetAttribute("id",std::to_string(entry.first));
			viewFactor->LinkEndChild(new TiXmlText(std::to_string(entry.second)));
			viewFactors->LinkEndChild(viewFactor);
		}
		e->LinkEndChild(viewFactors);
	}
	return e;
}

} // namespace VICUS
