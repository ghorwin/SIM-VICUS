/*	IBK Math Kernel Library
		Copyright (c) 2001-2016, Institut fuer Bauklimatik, TU Dresden, Germany

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
	unsigned int polySize = polygon.size();
	for(unsigned int i=0; i<polySize; ++i){
		IBK::Line otherLine(polygon[i], polygon[(i+1)%polySize]);

		if(line.intersects(otherLine, intersectionPoint))
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

} // namespace IBKMK
