/*	Copyright (c) 2001-2017, Institut für Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, H. Fechner, St. Vogelsang, A. Paepcke, J. Grunewald
	All rights reserved.

	This file is part of the IBK Library.

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


	This library contains derivative work based on other open-source libraries.
	See OTHER_LICENCES and source code headers for details.

*/

#include "IBK_Line.h"
#include "IBK_math.h"

namespace IBK {

Line::Line(const IBK::point2D<double> & p1, const IBK::point2D<double> & p2) :
	m_p1(p1),
	m_p2(p2)
{
}

Line::Line(double x1, double y1, double x2, double y2) :
	m_p1(x1, y1),
	m_p2(x2, y2)
{
}

bool similar(double x1, double y1, double x2, double y2, const double eps = 1e-6) {
	return (IBK::near_equal(x1, x2, eps) && IBK::near_equal(y1, y2, eps));
}


unsigned int Line::intersection(
	double ax1, double ay1, double ax2, double ay2,
	double bx1, double by1, double bx2, double by2,
	IBK::point2D<double> & p1, IBK::point2D<double> & p2)
{
	// solution is based on vector-variant
	//   Line 1: = p...p+r
	//   Line 2: = q...q+s
	//
	// Equate both line equations: p + t.r = q + u.s    // t and u are scalars
	//
	// Solve for t:
	//   (p + t.r) x s = (q + u.s) x s
	//   t.r x s = (q-p) x s                            // mind: s x s = 0
	//   t       = (q-p) x s / (r x s)
	//
	// Solve for u:
	//   (p + t.r) x r = (q + u.s) x r
	//   u             = (p-q) x r / (s x r)
	//   u             = (q-p) x r / (r x s)            // mind: s x r = - r x s

	// p = (ax1,ay1)
	// r = (ax2-ax1, ay2-ay1)
	// q = (bx1, by1)
	// s = (bx2-bx1, by2-by1)

	double rx = ax2-ax1;
	double ry = ay2-ay1;
	double sx = bx2-bx1;
	double sy = by2-by1;

	// zero line length check
	IBK_ASSERT(rx*rx + ry*ry > 0);
	IBK_ASSERT(sx*sx + sy*sy > 0);

	// cross-product
	double rs = rx * sy - ry * sx;  // this is actually the z-component

	double qminuspx = bx1-ax1;
	double qminuspy = by1-ay1;

	// (q - p) x r
	double qminuspxr = qminuspx * ry - qminuspy* rx;

	// handle all cases:
	//
	// 1 : r x s == 0 : qminuspxr == 0 lines are collinear

#define EPS 1e-13

	double absrs = std::fabs(rs);
	double absqminuspxr = std::fabs(qminuspxr);
	if (absrs < 1e-8 && absqminuspxr < 1e-8 ) { // near zero

		// check where second line intersects first
		// Equal start point q and end points (q+s) with equation of line 1 and solve for t:
		//   t0 = (q − p) · r / (r · r)
		//   t1 = (q + s − p) · r / (r · r) = t0 + s · r / (r · r)
		//      = t0 + s · r / (r · r)

		double rdotr = rx*rx + ry*ry; // cannot be zero, since zero lines have been checked already

		double t0 = (qminuspx*rx + qminuspy*ry) / rdotr;
		double t1 = t0 + (sx*rx + sy*ry) / rdotr;
		if (t1 < t0)
			std::swap(t0, t1);

		// distinguish 6 cases

		// case a: 1----2----2-----1
		// case b: 1----2----1-----2
		// case c: 1----1----2-----2
		// case d: 2----1----1-----2
		// case e: 2----1----2-----1
		// case f: 2----2----1-----1

		// Note: case c and d have no intersections
		//       case b and case e can have the special case that the intersectionn point in the middle is the same

		// case c: separate
		if (t0 > 1 + EPS)
			return 0;

		// case c: connected
		if (t0 > 1 - EPS) {
			p1.m_x = bx1;
			p1.m_y = by1;
			return 1;
		}

		// case f: separate
		if (t1 < -EPS)
			return 0;
		// case f: connected
		if (t1 < EPS) {
			p1.m_x = bx2;
			p1.m_y = by2;
			return 1;
		}

		// case a
		if (t0 >= 0 && t1 <= 1) {
			// line 2 is in the middle
			p1.m_x = bx1;
			p1.m_y = by1;
			p2.m_x = bx2;
			p2.m_y = by2;
			return 2; // we have two segment points
		}

		// case b
		if (t0 > 1 && t1 > 1) {
			p1.m_x = ax1 + t0*rx;
			p1.m_y = ay1 + t0*ry;
			p2.m_x = bx1;
			p2.m_y = by1;

			if (p1.m_x == p2.m_x && p1.m_y == p2.m_y)
				return 1;
			return 2;
		}

		// case d:
		if (t0 <= 0 && t1 >= 1) {
			// line 1 is in the middle or is the same as line 2
			p1.m_x = ax1;
			p1.m_y = ay1;
			p2.m_x = ax2;
			p2.m_y = ay2;
			return 2;
		}

		// case e:
		if (t0 < 0 && t1 < 1) {
			p1.m_x = ax1;
			p1.m_y = ay1;
			p2.m_x = ax1 + t1*rx;
			p2.m_y = ay1 + t1*ry;
			if (p1.m_x == p2.m_x && p1.m_y == p2.m_y)
				return 1;
			return 2;
		}

	}

	// 2 : r x s == 0 : qminuspxr != 0 lines are parallel

	if (absrs < 1e-8 && absqminuspxr >= 1e-8 ) {
		return 0;
	}

	// 3 : r x s != 0 lines intersect

	// both lines are not parallel, compute intersection point
	// solve for intersection point
	// (p + t r) × s = (q + u s) × s
	//
	// since s x s = 0:
	//
	// t (r × s) = (q − p) × s
	//
	// t = (q − p) × s / (r × s)
	// and
	// u = (q − p) × r / (r × s)

	double qminuspxs = qminuspx * sy - qminuspy* sx;
	double t = qminuspxs / rs;
	double u = qminuspxr / rs;
	// check if u and t are with 0 and 1

	if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {

		p1.m_x = bx1 + u*sx;
		p1.m_y = by1 + u*sy;
		return 1;
	}

	return 0; // lines do not intersect
}

} // namespace IBK


