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

#ifndef VICUS_NetworkLineH
#define VICUS_NetworkLineH

#include "VICUS_NetworkEdge.h"

#include <IBKMK_Vector3D.h>

namespace VICUS {

/*! Line is a helper class that implements a range of line operations.
	It is meant to be constructed from an edge definition.
	It stores the line internally in vector form a*lambda + b.
*/

extern const double	NetworkGeometricResolution;		/// geometric resolution in m, points closer than that are assumed equal

class NetworkLine {
public:

	NetworkLine(const IBKMK::Vector3D &p1, const IBKMK::Vector3D &p2):
		m_a(p1),
		m_b(p2-p1)
	{}

	NetworkLine(const NetworkEdge &e):
		m_a(e.m_node1->m_position),
		m_b(e.m_node2->m_position - e.m_node1->m_position)
	{}

	/*! return intersection point between two lines */
	void intersection(const NetworkLine &line, IBKMK::Vector3D & pInter) const;

	/*! returns distance between point and line, if point is not contained by line, the distance to the closest line endpoint is returned */
	double distanceToPoint(const IBKMK::Vector3D & point) const;

	/*! determines wether the given point is on the line, between the determining points but does not match any of the determining points */
	bool containsPoint(const IBKMK::Vector3D &point) const;

	/*! length of line */
	double length() const;

	/*! retruns distance between two given points */
	static double distanceBetweenPoints(const IBKMK::Vector3D & point1, const IBKMK::Vector3D & point2);

	/*! checks wether the distance between two points is below the threshold */
	static bool pointsMatch(const IBK::point2D<double> &point1, const IBK::point2D<double> &point2);

	/*! Offset vector of line */
	IBKMK::Vector3D			m_a;
	/*! Direction vector of line */
	IBKMK::Vector3D			m_b;

};

} // namespace VICUS

#endif // VICUS_NetworkLineH
