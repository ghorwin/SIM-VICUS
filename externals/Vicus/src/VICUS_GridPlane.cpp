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

#include "VICUS_GridPlane.h"

#include <IBK_Exception.h>
#include <IBK_messages.h>

#include <IBKMK_3DCalculations.h>

namespace VICUS {

GridPlane::GridPlane(const IBKMK::Vector3D & offset, const IBKMK::Vector3D & normal, const IBKMK::Vector3D & localX,
		  const QColor majorGridColor, double width, double majorGridSpacing) :
	m_offset(offset),
	m_normal(normal),
	m_localX(localX),
	m_color(majorGridColor),
	m_width(width),
	m_spacing(majorGridSpacing)
{
	FUNCID(GridPlane::GridPlane);
	try {
		updateLocalY();
	} catch (...) {
		IBK::IBK_Message("Invalid localX and/or normal vectors.", IBK::MSG_ERROR, FUNC_ID);
	}
}

void GridPlane::readXML(const TiXmlElement * element) {
	readXMLPrivate(element);
	updateLocalY();
}


TiXmlElement * GridPlane::writeXML(TiXmlElement * parent) const {
	return writeXMLPrivate(parent);
}


bool GridPlane::operator!=(const GridPlane & other) const {
	if (m_isVisible != other.m_isVisible) return true;
	if (m_isActive != other.m_isActive) return true;
	if (m_color != other.m_color) return true;
	if (m_width != other.m_width) return true;
	if (m_spacing != other.m_spacing) return true;
	if (m_offset != other.m_offset) return true;
	if (m_normal != other.m_normal) return true;
	if (m_localX != other.m_localX) return true;
	if (m_name != other.m_name) return true;
	return false;
}


void GridPlane::updateLocalY() {
	FUNCID(GridPlane::updateLocalY);
	// also update localY axis and check for valid vectors
	if (!IBK::nearly_equal<6>(m_normal.magnitudeSquared(), 1.0))
		throw IBK::Exception("Normal vector does not have unit length!", FUNC_ID);
	if (!IBK::nearly_equal<6>(m_localX.magnitudeSquared(), 1.0))
		throw IBK::Exception("xAxis vector does not have unit length!", FUNC_ID);
	// check that the vectors are (nearly) orthogonal
	double sp = m_normal.scalarProduct(m_localX);
	if (!IBK::nearly_equal<6>(sp, 0.0))
		throw IBK::Exception("Normal and xAxis vectors must be orthogonal!", FUNC_ID);

	// we only modify our vectors if all input data is correct - hence we ensure validity of the polygon
	m_normal.crossProduct(m_localX, m_localY); // Y = N x X - right-handed coordinate system
	m_localY.normalize();
}


bool GridPlane::intersectsLine(const IBKMK::Vector3D & p, const IBKMK::Vector3D & direction,
									  double & t,
									  IBKMK::Vector3D & intersectionPoint) const
{
	if (!m_isActive || !m_isVisible)
		return false;

	FUNCID(VICUS::GridPlane::intersectsLine);
	if (!IBK::nearly_equal<6>(m_normal.magnitudeSquared(), 1.0))
		throw IBK::Exception("Normal vector does not have unit length!", FUNC_ID);

	double d_dot_normal = direction.scalarProduct(m_normal);
	double angle = d_dot_normal/direction.magnitude();
	// line parallel to plane? no intersection
	if (angle < 1e-8 && angle > -1e-8)
		return false;

	// Condition 1: same direction of normal vectors?
	// -> does not apply to grid planes

	t = (m_offset - p).scalarProduct(m_normal) / d_dot_normal;

	// Condition 2: outside viewing range?
	if (t < 0 || t > 1)
		return false;

	// now determine location on plane
	IBKMK::Vector3D x0 = p + t*direction;

	// plane is endless - return intersection point and normalized distance t (no hole checking here!)
	intersectionPoint = x0;

	// determine snap point
	return true;
}


void GridPlane::closestSnapPoint(const IBKMK::Vector3D & intersectionPoint, IBKMK::Vector3D & snapPoint) const {
	FUNCID(GridPlane::closestSnapPoint);
	if (m_nGridLines == 0)
		throw IBK::Exception("GridPlane not properly initialized.", FUNC_ID);

	// Note: we rely on valid m_localX, m_localY and m_normal vectors, all orthogonal and with magnitude 1

	IBK_ASSERT(m_nGridLines % 2 == 1); // must be an odd number!

	// Indexes of the grid lines, -(m_nGridLines-1)/2 ... +(m_nGridLines-1)/2
	int i = 0;
	int j = 0;
	int maxGridLineNum = (m_nGridLines-1)/2;

	// we first compute the projected plain coordinates
	double localX, localY;

	// special handling for horizontal grids (the vast majority of grids will be this way)
	if (m_normal.m_x == 0.0 && m_normal.m_y == 0.0) {
		localX = intersectionPoint.m_x;
		localY = intersectionPoint.m_y;
	}
	else {
		if (!planeCoordinates(m_offset, m_localX, m_localY,
							  intersectionPoint, localX, localY))
		{
			snapPoint = intersectionPoint;
			return;
		}
	}

	// now we check if we are outside our plane limits and clip the coordinates respectively
	double minorGridSpacing = m_spacing/10;
	if (localX < -m_gridExtends) {
		i = -maxGridLineNum;
	}
	else if (localX > m_gridExtends) {
		i = maxGridLineNum;
	}
	else {
		i = (int)std::floor(intersectionPoint.m_x / minorGridSpacing + 0.5);  // round real (up/down)
	}

	if (localY < -m_gridExtends) {
		j = -maxGridLineNum;
	}
	else if (localY > m_gridExtends) {
		j = maxGridLineNum;
	}
	else {
		j = (int)std::floor(intersectionPoint.m_y / minorGridSpacing + 0.5);  // round real (up/down)
	}

	// recompute snap coordinates
	snapPoint = m_offset + i*m_localX + j*m_localY;
}


} // namespace VICUS
