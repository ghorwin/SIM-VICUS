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
static bool eleminateColinearPtsHelper(std::vector<IBKMK::Vector2D> &polyline) {

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

void eleminateColinearPoints(std::vector<IBKMK::Vector2D> & polygon) {
	if(polygon.size()<2)
		return;
	//check for duplicate points in polyline and remove duplicates
	for (int i=(int)polygon.size()-1; i>=0; --i) {
		if(polygon.size()<2)
			return;
		size_t j=(size_t)i-1;
		if(i==0)
			j=polygon.size()-1;
		if((polygon[(size_t)i]-polygon[j]).magnitude()<0.001)
			polygon.erase(polygon.begin()+i);
	}

	bool tryAgain =true;
	while (tryAgain)
		tryAgain = eleminateColinearPtsHelper(polygon);
}

} // namespace IBKMK
