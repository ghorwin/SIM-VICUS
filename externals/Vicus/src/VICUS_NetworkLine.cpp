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

#include "VICUS_NetworkLine.h"

#include <IBKMK_3DCalculations.h>

#include <algorithm>
#include <cmath>

namespace VICUS {

extern const double	GeometricResolution = 0.01; // in m

void NetworkLine::intersection(const NetworkLine &line, IBKMK::Vector3D &pInter) const {
	double l1, l2;
	lineToLineDistance(m_a, m_b, line.m_a, line.m_b, l1, pInter, l2);
}


double NetworkLine::distanceToPoint(const IBKMK::Vector3D &point) const{
	double lineFactor = 0;
	IBKMK::Vector3D projectionPoint;
	double distance = IBKMK::lineToPointDistance(m_a, m_b, point, lineFactor, projectionPoint);
	// if point is outside line, we calculate the distance to the closest endpoint
	if (lineFactor > 1 || lineFactor < 0) {
		double distanceA = (m_a - point).magnitude();
		double distanceB = ((m_a + m_b) - point).magnitude();
		distance = std::min(distanceA, distanceB);
	}

	return distance;
}


bool NetworkLine::containsPoint(const IBKMK::Vector3D &point) const {
	double lineFactor = 0;
	IBKMK::Vector3D projectionPoint;
	double dist = IBKMK::lineToPointDistance(m_a, m_b, point, lineFactor, projectionPoint);
	return dist < GeometricResolution && IBK::near_gr(lineFactor, 0) && IBK::near_le(lineFactor, 1);
}


double NetworkLine::length() const {
	return m_b.magnitude();
}

double NetworkLine::distanceBetweenPoints(const IBKMK::Vector3D &point1, const IBKMK::Vector3D &point2) {
	return IBKMK::Vector3D(point2 - point1).magnitude();
}



} // namespace VICUS
