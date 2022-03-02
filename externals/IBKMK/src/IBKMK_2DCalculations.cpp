/*	IBK Math Kernel Library
	Copyright (c) 2001-today, Institut fuer Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, A. Paepcke, H. Fechner, St. Vogelsang
	All rights reserved.

	This file is part of the IBKMK Library.

	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
	   list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation
	   and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors
	   may be used to endorse or promote products derived from this software without
	   specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
	ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	This library contains derivative work based on other open-source libraries,
	see LICENSE and OTHER_LICENSES files.

*/

#include "IBKMK_2DCalculations.h"
#include "IBK_Line.h"

namespace IBKMK {

bool intersectsLine2D(const std::vector<Vector2D> & polygon,
					  const IBK::point2D<double> &p1, const IBK::point2D<double> &p2, IBK::point2D<double> & intersectionPoint)
{
	IBK::Line line(p1, p2);
	unsigned int polySize = (unsigned int)polygon.size();
	for (unsigned int i=0; i<polySize; ++i){
		IBK::Line otherLine(polygon[i], polygon[(i+1)%polySize]);

		// TODO : enhance intersectsLine2D  function to return also two intersection points
		IBK::point2D<double> intersectionPoint2;
		if (line.intersects(otherLine, intersectionPoint, intersectionPoint2) > 0)
			return true;
	}
	return false;
}


static int crossProdTest(const IBKMK::Vector2D & a, IBKMK::Vector2D b, IBKMK::Vector2D c) {

	if (a.m_y == b.m_y && a.m_y == c.m_y) {
		if (	(b.m_x<= a.m_x && a.m_x <= c.m_x) ||
				(c.m_x<= a.m_x && a.m_x <= b.m_x) )
			return 0;
		else
			return 1;
	}

	if (b.m_y > c.m_y)
		std::swap(b,c);

	if (a.m_y <= b.m_y || a.m_y > c.m_y)
		return 1;

	double delta = (b.m_x - a.m_x) * (c.m_y - a.m_y) -(b.m_y - a.m_y) * (c.m_x - a.m_x);
	if(delta > 0)			return	1;
	else if(delta < 0)		return	-1;
	else					return	0;
}


/* Point in Polygon function. Result:
	-1 point not in polyline
	0 point on polyline
	1 point in polyline

	\param	point test point
	Source https://de.wikipedia.org/wiki/Punkt-in-Polygon-Test_nach_Jordan

*/
int pointInPolygon(const std::vector<Vector2D> & polygon, const IBK::point2D<double> &p) {
	int t=-1;
	size_t polySize = polygon.size();
	for (size_t i=0; i<polySize; ++i) {
		t *= crossProdTest(p, polygon[i], polygon[(i+1) % polySize]);
		if (t==0)
			break;
	}

	return  t;
}


void eliminateCollinearPoints(std::vector<IBKMK::Vector2D> & polygon, double epsilon) {
	if (polygon.size()<2)
		return;

	// check for duplicate points in polyline and remove duplicates

	// the algorithm works as follows:
	// - we start at current index 0
	// - we compare the vertex at current index with that of the next vertex
	//    - if both are close enough together, we elminate the current vertex and try again
	//    - otherwise both vertexes are ok, and we advance the current index
	// - algorithm is repeated until we have processed the last point of the polygon
	// Note: when checking the last point of the polygon, we compare it with the first vertex (using modulo operation).
	unsigned int i=0;
	double eps2 = epsilon*epsilon;
	while (polygon.size() > 1 && i < polygon.size()) {
		// distance between current and next point
		IBKMK::Vector2D diff = polygon[i] - polygon[(i+1) % polygon.size()]; // Note: when i = size-1, we take different between last and first element
		if (diff.magnitudeSquared() < eps2)
			polygon.erase(polygon.begin()+i); // remove point and try again
		else
			++i;
	}

	// Now we have only different points. We process the polygon again and remove all vertexes between collinear edges.
	// The algorithm uses the following logic:
	//    - take 3 subsequent vertexes, compute lines from vertex 1-2 and 1-3
	//    - compute projection of line 1 onto 2
	//    - compute projected end point of line 1 in line 2
	//    - if distance between projected point and original vertex 2 is < epsison, remove vertex 2

	i=0;
	while (polygon.size() > 1 && i < polygon.size()) {
		// we check if we can remove the current vertex i
		// take the last and next vertex
		const IBKMK::Vector2D & last = polygon[(i + polygon.size() - 1) % polygon.size()];
		const IBKMK::Vector2D & next = polygon[(i+1) % polygon.size()];
		// compute vertex a and b and normalize
		IBKMK::Vector2D a = next - last; // vector to project on
		IBKMK::Vector2D b = polygon[i] - last; // vector that shall be projected
		double anorm2 = a.magnitudeSquared();
		if (anorm2 < eps2) {
			// next and last vectors are nearly identical, hence we have a "spike" geometry and need to remove
			// both the spike and one of the identical vertexes
			polygon.erase(polygon.begin()+i); // remove point (might be the last)
			if (i < polygon.size())
				polygon.erase(polygon.begin()+i); // not the last point, remove next as well
			else
				polygon.erase(polygon.begin()+i-1); // remove previous point
			continue;
		}

		// compute projection vector
		IBKMK::Vector2D astar = a.scalarProduct(b)/anorm2 * a;
		// get vertex along vector b
		astar += last;
		// compute difference
		IBKMK::Vector2D diff = polygon[i] - astar;
		if (diff.magnitudeSquared() < eps2)
			polygon.erase(polygon.begin()+i); // remove point and try again
		else
			++i;
	}
}


void enlargeBoundingBox(const Vector2D & v, Vector2D & minVec, Vector2D & maxVec) {
	minVec.m_x = std::min(minVec.m_x, v.m_x);
	minVec.m_y = std::min(minVec.m_y, v.m_y);

	maxVec.m_x = std::max(maxVec.m_x, v.m_x);
	maxVec.m_y = std::max(maxVec.m_y, v.m_y);
}


} // namespace IBKMK
