/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  ... all the others from the SIM-VICUS team ... :-)

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#include "VICUS_NetworkLine.h"

#include <algorithm>
#include <cmath>

namespace VICUS {

extern const double	geometricResolution = 0.01; // in m

void NetworkLine::intersection(const NetworkLine &line, IBK::point2D<double> &pInter) const
{
	const double &x1 = m_p1.m_x; const double &x2 = m_p2.m_x; const double &x3 = line.m_p1.m_x; const double &x4 = line.m_p2.m_x;
	const double &y1 = m_p1.m_y; const double &y2 = m_p2.m_y; const double &y3 = line.m_p1.m_y; const double &y4 = line.m_p2.m_y;
	pInter.m_x = ( (x4 - x3) * (x2 * y1 - x1 * y2) - (x2 - x1) * (x4 * y3 - x3 * y4) ) / ( (y4 - y3) * (x2 - x1) - (y2 - y1) * (x4 - x3) );
	pInter.m_y = ( (y1 - y2) * (x4 * y3 - x3 * y4) - (y3 - y4) * (x2 * y1 - x1 * y2) ) / ( (y4 - y3) * (x2 - x1) - (y2 - y1) * (x4 - x3) );
}


void NetworkLine::projectionFromPoint(const IBK::point2D<double> &point, IBK::point2D<double> &proj) const{
	// vector form g = a + s*b;  with a = (m_p1.m_x, m_p1.m_y)
	double b1 = m_p2.m_x - m_p1.m_x;
	double b2 = m_p2.m_y - m_p1.m_y;
	double s = (point.m_x + b2/b1*point.m_y - m_p1.m_x - b2/b1*m_p1.m_y) / (b1 + b2*b2/b1);
	proj.m_x = m_p1.m_x + s*b1;
	proj.m_y = m_p1.m_y + s*b2;
}


double NetworkLine::distanceToPoint(const IBK::point2D<double> &point) const{
	IBK::point2D<double> proj;
	projectionFromPoint(point, proj);
	if (containsPoint(proj))
		return distanceBetweenPoints(point, proj);
	else
		return std::min(distanceBetweenPoints(m_p1, point), distanceBetweenPoints(m_p2, point));
}


bool NetworkLine::containsPoint(const IBK::point2D<double> &point) const
{
	bool inside = (point.m_x >= std::min(m_p1.m_x, m_p2.m_x)) && (point.m_x <= std::max(m_p1.m_x, m_p2.m_x)) &&
			(point.m_y >= std::min(m_p1.m_y, m_p2.m_y)) && (point.m_y <= std::max(m_p1.m_y, m_p2.m_y));
	bool identity = distanceBetweenPoints(point, m_p1) < geometricResolution ||
			distanceBetweenPoints(point, m_p2) < geometricResolution ;
	return inside && !identity;
}


double NetworkLine::length() const
{
	return distanceBetweenPoints(m_p1, m_p2);
}

double NetworkLine::distanceBetweenPoints(const IBK::point2D<double> &point1, const IBK::point2D<double> &point2)
{
	return std::sqrt( (point1.m_x-point2.m_x) * (point1.m_x-point2.m_x) + (point1.m_y-point2.m_y) * (point1.m_y-point2.m_y) );
}



} // namespace VICUS
