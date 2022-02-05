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

#include <set>

#include <QLineF>
#include <QPolygonF>

#include <IBK_Line.h>
#include <IBK_math.h>
#include <IBK_messages.h>

#include <IBKMK_3DCalculations.h>

#include <NANDRAD_Utilities.h>


#include <VICUS_Constants.h>
#include <VICUS_KeywordList.h>

#include <tinyxml.h>

namespace VICUS {


bool Polygon3D::operator!=(const Polygon3D & other) const {
	// 3D polygon is equal when polylines match and all three vectors are the same
	if (m_normal != other.m_normal)
		return true;
	if (m_localX != other.m_localX)
		return true;
	if (m_offset != other.m_offset)
		return true;

	if (m_polyline != other.m_polyline)
		return true;
	return false;
}


void Polygon3D::readXML(const TiXmlElement * element) {
	readXMLPrivate(element); // when Polygon2D is read, it will be checked for validity already

	// now check the normal vector and xAxis and update the yAxis
	// also marks the 3D vertexes as "dirty"
	setRotation(m_normal, m_localX); // Might throw an exception if validity check fails
}


TiXmlElement * Polygon3D::writeXML(TiXmlElement * parent) const {
	if (*this == Polygon3D())
		return nullptr;

	return writeXMLPrivate(parent);
}


bool Polygon3D::isValid() const {
	if (m_normal == IBKMK::Vector3D())
		return false; // never set a valid plane orientation
	return m_polyline.isValid();
}


void Polygon3D::setRotation(const IBKMK::Vector3D & normal, const IBKMK::Vector3D & xAxis) {
	FUNCID(Polygon3D::setRotation);

	if (!IBK::nearly_equal<6>(normal.magnitudeSquared(), 1.0))
		throw IBK::Exception("Normal vector does not have unit length!", FUNC_ID);
	if (!IBK::nearly_equal<6>(xAxis.magnitudeSquared(), 1.0))
		throw IBK::Exception("xAxis vector does not have unit length!", FUNC_ID);
	// check that the vectors are (nearly) orthogonal
	double sp = normal.scalarProduct(xAxis);
	if (!IBK::nearly_equal<6>(sp, 1.0))
		throw IBK::Exception("Normal and xAxis vectors must be orthogonal!", FUNC_ID);

	// we only modify our vectors if all input data is correct - hence we ensure validity of the polygon
	m_normal = normal;
	m_localX = xAxis;
	normal.crossProduct(xAxis, m_localY); // Y = N x X - right-handed coordinate system
	m_localY.normalize();
	m_dirty = true; // mark 3D vertexes as dirty
}


IBKMK::Vector3D Polygon3D::centerPoint() const {
	FUNCID(Polygon3D::centerPoint);
	if (!isValid())
		throw IBK::Exception("Invalid polygon.", FUNC_ID);

	size_t counter=0;
	IBKMK::Vector3D vCenter;

	for (const IBKMK::Vector3D & v : vertexes()) {
		vCenter += v;
		++counter;
	}
	vCenter/=static_cast<double>(counter);

	return vCenter;
}


const std::vector<IBKMK::Vector3D> & Polygon3D::vertexes() const {
	if (m_dirty) {
		// re-compute 3D vertex coordinates
		unsigned int vertexCount = m_polyline.vertexes().size();
		m_vertexes.resize(vertexCount);
		for (unsigned int i=0; i<vertexCount; ++i) {
			const IBKMK::Vector2D & v2 = m_polyline.vertexes()[i];
			m_vertexes[i] = m_offset + v2.m_x*m_localX + v2.m_y*m_localY;
		}
		m_dirty = false;
	}
	return m_vertexes;
}


} // namespace VICUS

