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

		if (line.intersects(otherLine, intersectionPoint))
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


// Eleminate one coolinear point. If a point is erased return true.
static bool eliminateCollinearPtsHelper(std::vector<IBKMK::Vector2D> &polyline) {

	if(polyline.size()<=2)
		return false;

	const double eps = 1e-4;
	unsigned int polySize = (unsigned int)polyline.size();

	for(unsigned int idx=0; idx<polySize; ++idx){
		unsigned int idx0 = idx-1;
		if(idx==0)
			idx0 = polySize-1;

		IBKMK::Vector2D a = polyline.at(idx0) - polyline.at(idx);
		IBKMK::Vector2D b = polyline.at((idx+1) % polySize) - polyline.at(idx);
		a.normalize();
		b.normalize();

		double cosAngle = a.scalarProduct(b);


		if(cosAngle < -1+eps || cosAngle > 1-eps){
			polyline.erase(polyline.begin()+idx);
			return true;
		}
	}
	return false;
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

	// Now we have only different points. We process the polygon again and compute the angle between all
	// polygon edges and remove all vertexes between collinear edges
	// TODO : the algorithm can be reworked to use the following logic:
	//        - take 3 subsequent vertexes, compute lines from vertex 1-2 and 1-3
	//        - compute projection of line 1 onto 2
	//        - compute projected end point of line 1 in line 2
	//        - if distance between projected point and original vertex 2 is < epsison, remove vertex 2

	bool tryAgain =true;
	while (tryAgain)
		tryAgain = eliminateCollinearPtsHelper(polygon);
}


} // namespace IBKMK
