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

#ifndef VICUS_NetworkLineH
#define VICUS_NetworkLineH

#include "VICUS_NetworkEdge.h"

#include <IBK_point.h>

namespace VICUS {

/*! Line is a helper class that implements a range of 2D line operations.
	It is meant to be constructed from an edge definition, and caches
	the respective node coordinates, internally.
*/

extern const double	geometricResolution;		/// geometric resolution in m, points closer than that are assumed equal

class NetworkLine {
public:

	NetworkLine(const IBK::point2D<double> &p1, const IBK::point2D<double> &p2):
		m_p1(p1),
		m_p2(p2)
	{}

	NetworkLine(const NetworkEdge &e):
		m_p1(e.m_node1->m_position.m_x, e.m_node1->m_position.m_y),
		m_p2(e.m_node2->m_position.m_x, e.m_node2->m_position.m_y)
	{}

	/*! return intersection point between two lines */
	void intersection(const NetworkLine &line, IBK::point2D<double> &pInter) const;

	/*! return othogonal projection of point on line */
	void projectionFromPoint(const IBK::point2D<double> &point, IBK::point2D<double> &pProj) const;

	/*! return orthogonal distance between point and line */
	double distanceToPoint(const IBK::point2D<double> &point) const;

	/*! determines wether the given point is on the line, between the determining points but does not match any of the determining points */
	bool containsPoint(const IBK::point2D<double> &point) const;

	/*! length of line */
	double length() const;

	/*! retruns distance between two given points */
	static double distanceBetweenPoints(const IBK::point2D<double> &point1, const IBK::point2D<double> &point2);

	/*! checks wether the distance between two points is below the threshold */
	static bool pointsMatch(const IBK::point2D<double> &point1, const IBK::point2D<double> &point2);

	IBK::point2D<double>	m_p1;
	IBK::point2D<double>	m_p2;

};

} // namespace VICUS

#endif // VICUS_NetworkLineH
