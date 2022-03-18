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

#include "VICUS_Polygon3D.h"

#include <VICUS_Constants.h>
#include <VICUS_KeywordList.h>

#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace VICUS {

void Polygon3D::readXML(const TiXmlElement * element) {
	FUNCID(Polygon3D::readXML);

	// if element has child "offset", then we have the new format
	const TiXmlAttribute * attrib = TiXmlAttribute::attributeByName(element, "offset");
	if (attrib != nullptr) {
		IBKMK::Vector3D offset, normal, localX;
		try {
			offset = IBKMK::Vector3D::fromString(attrib->ValueStr());
		} catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, "Error reading 'offset' vector from 'Polygon3D' tag.", FUNC_ID);
		}

		attrib = TiXmlAttribute::attributeByName(element, "normal");
		if (attrib == nullptr)
			throw IBK::Exception("Missing attribute 'normal' in 'Polygon3D' tag.", FUNC_ID);
		try {
			normal = IBKMK::Vector3D::fromString(attrib->ValueStr());
		} catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, "Error reading 'normal' vector from 'Polygon3D' tag.", FUNC_ID);
		}

		attrib = TiXmlAttribute::attributeByName(element, "localX");
		if (attrib == nullptr)
			throw IBK::Exception("Missing attribute 'localX' in 'Polygon3D' tag.", FUNC_ID);
		try {
			localX = IBKMK::Vector3D::fromString(attrib->ValueStr());
		} catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, "Error reading 'localX' vector from 'Polygon3D' tag.", FUNC_ID);
		}

		// read vertexes
		std::string text = element->GetText();
		text = IBK::replace_string(text, ",", " ");
		std::vector<IBKMK::Vector2D> verts;
		try {
			std::vector<double> vals;
			IBK::string2valueVector(text, vals);
			// must have n*2 elements
			if (vals.size() % 2 != 0)
				throw IBK::Exception("Mismatching number of values.", FUNC_ID);
			if (vals.empty())
				throw IBK::Exception("Missing values.", FUNC_ID);
			verts.resize(vals.size() / 2);
			for (unsigned int i=0; i<verts.size(); ++i){
				verts[i].m_x = vals[i*2];
				verts[i].m_y = vals[i*2+1];
			}
			IBKMK::Polygon2D poly2D(verts);
			if (!poly2D.isValid())
				throw IBK::Exception("Invalid polyline.", FUNC_ID);

			// now create the polygon 3D
			IBKMK::Polygon3D poly3D(verts, offset, normal, localX);
			if (!poly3D.isValid())
				throw IBK::Exception("Invalid polygon definition.", FUNC_ID);

			// Note: a VICUS::Polyon3D _is a_ IBKMK::Polygon3D, so we can cast them into each other
			IBK_ASSERT(dynamic_cast<IBKMK::Polygon3D*>(this) != nullptr);
			dynamic_cast<IBKMK::Polygon3D&>(*this) = poly3D;
		} catch (IBK::Exception & ex) {
			throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row())
								  .arg("Error reading element 'Polygon3D'." ), FUNC_ID);
		}
	}
	else {
		// try reading in old format, first
		try {
			// we may have a type attribute
			IBKMK::Polygon2D::type_t t = IBKMK::Polygon2D::T_Polygon;
			const TiXmlAttribute * attrib = TiXmlAttribute::attributeByName(element, "type");
			if (attrib != nullptr) {
				if (attrib->ValueStr() == "Rectangle")
					t = IBKMK::Polygon2D::T_Rectangle;
				else if (attrib->ValueStr() == "Triangle")
					t = IBKMK::Polygon2D::T_Triangle;
			}

			std::vector<IBKMK::Vector3D> verts;
			NANDRAD::readVector3D(element, "Polygon3D", verts);

			// Note: a VICUS::Polyon3D _is a_ IBKMK::Polygon3D, so we can cast them into each other
			IBK_ASSERT(dynamic_cast<IBKMK::Polygon3D*>(this) != nullptr);
			switch (t) {
				case IBKMK::Polygon2D::T_Triangle:
				case IBKMK::Polygon2D::T_Rectangle:
					if (verts.size() != 3)
						throw IBK::Exception("Invalid number of vertexes.", FUNC_ID);
					dynamic_cast<IBKMK::Polygon3D&>(*this) = IBKMK::Polygon3D(t, verts[0], verts[1], verts[2]);
				break;
				case IBKMK::Polygon2D::T_Polygon:
					dynamic_cast<IBKMK::Polygon3D&>(*this) = IBKMK::Polygon3D(verts);
				break;
				case IBKMK::Polygon2D::NUM_T: ; // just to make compiler happy
			}

		} catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, "Error reading 'Polygon3D' tag in old Vicus format.", FUNC_ID);
		}
	}
}


TiXmlElement * Polygon3D::writeXML(TiXmlElement * parent) const {
	if (*this == Polygon3D())
		return nullptr;

	TiXmlElement * e = new TiXmlElement("Polygon3D");
	parent->LinkEndChild(e);

	// encode vectors
	e->SetAttribute("offset", offset().toString());
	e->SetAttribute("normal", normal().toString());
	e->SetAttribute("localX", localX().toString());

	std::stringstream vals;
	const std::vector<IBKMK::Vector2D> & polyVertexes = polyline().vertexes();
	for (unsigned int i=0; i<polyVertexes.size(); ++i) {
		vals << polyVertexes[i].m_x << " " << polyVertexes[i].m_y;
		if (i<polyVertexes.size()-1)  vals << ", ";
	}
	TiXmlText * text = new TiXmlText( vals.str() );
	e->LinkEndChild( text );
	return e;
}


// Comparison operator !=
bool Polygon3D::operator!=(const Polygon3D &other) const {
	return IBKMK::Polygon3D::operator!=(other);
}


} // namespace VICUS

