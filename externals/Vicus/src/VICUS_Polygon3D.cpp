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
#include <IBKMK_Polygon3D.h>

#include <NANDRAD_Utilities.h>


#include <VICUS_Constants.h>
#include <VICUS_KeywordList.h>

#include <tinyxml.h>

namespace VICUS {


Polygon3D::Polygon3D(const Polygon2D & polyline, const IBKMK::Vector3D & normal,
					 const IBKMK::Vector3D & localX, const IBKMK::Vector3D & offset) :
	m_offset(offset), m_polyline(polyline)
{
	// guard against invalid polylines
	if (!polyline.isValid())
		m_polyline.clear();
	else
		setRotation(normal, localX);
}


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


Polygon3D Polygon3D::from3DVertexes(const std::vector<IBKMK::Vector3D> & vertexes) {
	FUNCID(Polygon3D::from3DVertexes);
	IBKMK::Polygon3D p(vertexes);
	if (!p.isValid())
		throw IBK::Exception("Invalid vertex vector, cannot construct valid polygon.", FUNC_ID);
	// a valid polygon has at least 1 vertex and valid normal and localX vectors
	Polygon3D newPoly = VICUS::Polygon3D((const VICUS::Polygon2D&)p.polyline(), p.normal(), p.localX(), p.vertexes()[0]);
	return newPoly;
}


#if 0
void Surface::healGeometry(const std::vector<IBKMK::Vector3D> &poly3D) {
	// we take a vector to hold our deviations, i.e. the sum of the vertical deviations from the plane.
	std::vector<double> deviations (poly3D.size(), 0);
	// create a vector to hold the projected points for each of the plane variants
	std::vector<std::vector<IBKMK::Vector3D> > projectedPoints ( poly3D.size(), std::vector<IBKMK::Vector3D> ( poly3D.size(), IBKMK::Vector3D (0,0,0) ) );

	// we iterate through all points and construct planes
	double smallestDeviation = std::numeric_limits<double>::max();
	unsigned int index = (unsigned int)-1;
	for (unsigned int i = 0, count = poly3D.size(); i<count; ++i ) {

		const IBKMK::Vector3D & offset = poly3D[i];

		const IBKMK::Vector3D & a = poly3D[(i + 1)         % count] - offset;
		const IBKMK::Vector3D & b = poly3D[(i - 1 + count) % count] - offset;

		// we find our plane
		// we now iterate again through all point of the polygon and
		for (unsigned int j = 0; j<count; ++j ) {

			if ( i == j ) {
				projectedPoints[i][j] = offset;
				continue;
			}

			// we take the current point
			const IBKMK::Vector3D & vertex = poly3D[j];

			// we find our projected points onto the plane
			double x, y;
			IBKMK::planeCoordinates(offset, a, b, vertex, x, y, 1e-2);

			// now we construct our projected points and find the deviation between the original points
			// and their projection
			projectedPoints[i][j] = offset + a*x + b*y;

			// add up the distance between original vertex and projected point
			// Note: if we add the square of the distances, we still get the maximum deviation, but avoid
			//       the expensive square-root calculation
			deviations[i] += (projectedPoints[i][j] - vertex).magnitudeSquared();
		}

		// determines smallest deviation
		if (deviations[i] < smallestDeviation) {
			index = i;
			smallestDeviation = deviations[i];
		}
	}

	// take the best vertex set and use it for the polygon
	setPolygon3D(projectedPoints[index]);
}

#endif


} // namespace VICUS

