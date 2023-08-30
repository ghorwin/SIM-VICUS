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

#include "IBKMK_Polygon2D.h"

#include <set>

#include <IBK_Line.h>
#include <IBK_math.h>
#include <IBK_messages.h>

#include "IBKMK_2DCalculations.h"

namespace IBKMK {

// this might also be a relative erro, for example based on the bounding box of the polygon.
// However, for use in SIM-VICUS we are happy with 0.1 mm accuracy?
const double TOLERANCE = 1e-4; // 0.1 mm should be fine enough


Polygon2D::Polygon2D(const std::vector<IBKMK::Vector2D> & vertexes) {
	setVertexes(vertexes);
}




// Comparison operator !=
bool Polygon2D::operator!=(const Polygon2D &other) const {
	if (m_type != other.m_type)
		return true;
	if (m_vertexes != other.m_vertexes)
		return true;
	return false;
}


bool Polygon2D::intersectsLine2D(const IBK::point2D<double> &p1, const IBK::point2D<double> &p2,
								 IBK::point2D<double> & intersectionPoint) const{
	return IBKMK::intersectsLine2D(m_vertexes, p1, p2, intersectionPoint);
}


void Polygon2D::setVertexes(const std::vector<IBKMK::Vector2D> & vertexes) {
	m_vertexes = vertexes;
	checkPolygon(); // if we have a triangle/rectangle, this is detected here; does not throw
}


void Polygon2D::boundingBox(Vector2D & lowerValues, Vector2D & upperValues) const {
	FUNCID(Polygon2D::boundingBox);
	if (m_vertexes.empty())
		throw IBK::Exception("Require at least one vertex in the polyline.", FUNC_ID);
	// initialize bounding box with first point
	lowerValues = m_vertexes[0];
	upperValues = m_vertexes[0];
	for (unsigned int i=1; i<m_vertexes.size(); ++i)
		IBKMK::enlargeBoundingBox(m_vertexes[i], lowerValues, upperValues);
}


double Polygon2D::areaSigned(int digits) const{
	FUNCID(Polygon2D::area);
	if(!m_valid)
		throw IBK::Exception("Invalid polygon.", FUNC_ID);
	double area = 0;
	unsigned int size = m_vertexes.size();
	for(unsigned int i=0, j = size-1; i<size; ++i){
		area += (m_vertexes[j].m_x + m_vertexes[i].m_x) * (m_vertexes[j].m_y - m_vertexes[i].m_y);
		j=i;
	}

	area *= -0.5;
	area = std::round(area*IBK::f_pow10(digits))/IBK::f_pow10(digits);

	return area;
}

double Polygon2D::area(int digits) const {
	FUNCID(Polygon2D::area);
	if (!m_valid)
		throw IBK::Exception("Invalid polygon.", FUNC_ID);
	double surfArea=0;
	unsigned int sizeV = m_vertexes.size();
	for (unsigned int i=0; i<sizeV; ++i){
		const IBKMK::Vector2D &p0 = m_vertexes[i];
		const IBKMK::Vector2D &p1 = m_vertexes[(i+1)%sizeV];
		const IBKMK::Vector2D &p2 = m_vertexes[(i+2)%sizeV];
		surfArea += p1.m_x * (p2.m_y - p0.m_y);

	}

	surfArea *= 0.5;
	surfArea = std::round(surfArea*IBK::f_pow10(digits))/IBK::f_pow10(digits);

	return std::fabs(surfArea);
}


double Polygon2D::circumference() const {
	FUNCID(Polygon2D::circumference);
	if (!m_valid)
		throw IBK::Exception("Invalid polygon.", FUNC_ID);
	double circumference=0;
	unsigned int sizeV = m_vertexes.size();
	for (unsigned int i=0; i<sizeV; ++i){
		const IBKMK::Vector2D &p0 = m_vertexes[i];
		const IBKMK::Vector2D &p1 = m_vertexes[(i+1)%sizeV];
		circumference += (p1-p0).magnitude();
	}
	return circumference;
}


bool Polygon2D::isSimplePolygon() const {
	std::vector<IBK::Line>	lines;
	for (unsigned int i=0, vertexCount = m_vertexes.size(); i<vertexCount; ++i) {
		lines.emplace_back(
					IBK::Line(
					IBK::point2D<double>(
								  m_vertexes[i].m_x,
								  m_vertexes[i].m_y),
					IBK::point2D<double>(
								  m_vertexes[(i+1) % vertexCount].m_x,
								  m_vertexes[(i+1) % vertexCount].m_y))
				);
	}
	if (lines.size()<4)
		return true;
	for (unsigned int i=0; i<lines.size();++i) {
		for (unsigned int j=0; j<lines.size()-2; ++j) {
			unsigned int k1 = (i+1)%lines.size();
			unsigned int k2 = (i-1);
			if(i==0)
				k2 = lines.size()-1;
			if(i==j || k1 == j || k2 == j )
				continue;
			//int k = (i+j+2)%lines.size();
			IBK::point2D<double> p;
			IBK::point2D<double> p2;
			if (lines[i].intersects(lines[j], p, p2) > 0)
				return false;
		}
	}

	return true;
}



// *** PRIVATE MEMBER FUNCTIONS ***

void Polygon2D::checkPolygon() {
	m_valid = false;
	if (m_vertexes.size() < 3)
		return;

	eleminateColinearPts();

	// we need 3 vertexes (not collinear) to continue
	if (m_vertexes.size() < 3)
		return;

	// try to simplify polygon to internal rectangle/parallelogram definition
	// this may change m_type to Rectangle or Triangle and subsequently speed up operations
	detectType();

	// polygon must not be winding into itself, otherwise triangulation would not be meaningful
	m_valid = isSimplePolygon();
}


void Polygon2D::detectType() {
	m_type = T_Polygon;
	if (m_vertexes.size() == 3) {
		m_type = T_Triangle;
		return;
	}
	if (m_vertexes.size() != 4)
		return;
	const IBKMK::Vector2D & a = m_vertexes[0];
	const IBKMK::Vector2D & b = m_vertexes[1];
	const IBKMK::Vector2D & c = m_vertexes[2];
	const IBKMK::Vector2D & d = m_vertexes[3];
	IBKMK::Vector2D c2 = b + (d-a);
	c2 -= c;
	// both points must within our acceptable tolerance
	if (c2.magnitude() < TOLERANCE)
		m_type = T_Rectangle;
}


void Polygon2D::eleminateColinearPts() {
	IBKMK::eliminateCollinearPoints(m_vertexes, TOLERANCE);
}


} // namespace IBKMK

