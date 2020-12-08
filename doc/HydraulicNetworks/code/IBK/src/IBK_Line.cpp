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

bool Line::intersection(
	double ax1, double ay1, double ax2, double ay2,
	double bx1, double by1, double bx2, double by2,
	IBK::point2D<double> & p)
{

	// compute dx1 and dx2
	double dx1 = ax1 - ax2;
	double dx2 = bx1 - bx2;
	double dy1 = ay1 - ay2;
	double dy2 = by1 - by2;
	double EPSILON = 1e-12;

	bool swapped = false;
	// swap x and y if dx1 less than given tolerance, i.e. when first line is vertical
	if (std::fabs(dx1) < EPSILON ) {
		std::swap(ax1, ay1);
		std::swap(ax2, ay2);
		std::swap(bx1, by1);
		std::swap(bx2, by2);
		std::swap(dx1, dy1);
		std::swap(dx2, dy2);
		swapped = true;
	}
	// the first line is not vertical, at least not when the second line wasn't vertical as well :-)

	// most-common cases first

	// test for special cases 1-3: both lines are parallel (horizontal/vertical) lines
	if (std::fabs(dy1) < EPSILON && std::fabs(dy2) < EPSILON) {
		// different y lines?
		if (std::fabs(ay1 - by1) >= EPSILON) {
			return false;
		}
		// ensure ax1 < ax2 and bx1 < bx2
		if (ax1 > ax2)		std::swap(ax1, ax2);
		if (bx1 > bx2)		std::swap(bx1, bx2);
		// both lines on same line, check if they share same points
		if (ax1 < bx1) {
			if (bx1 + EPSILON > ax2 && bx2 + EPSILON > ax2)
				return false; // Note: lines may share same end-point
			else
				return true; // lines overlap
		}
		else {
			if (ax1 + EPSILON > bx2 && ax2 + EPSILON > bx2)
				return false; // Note: lines may share same end-point
			else
				return true; // lines overlap
		}
	}

	// handle special cases where second line is vertical
	// (if first line was vertical, we swapped!)
	if (std::fabs(dx2) <= EPSILON) {
		// check if we catch x-interval
		if (std::min(bx1,bx2) + EPSILON > std::max(ax1,ax2) || std::max(bx1,bx2) - EPSILON < std::min(ax1,ax2) )
			return false;
		// compute intersection point directly by evaluating line equation
		double m1 = dy1/dx1;
		double y = ay2 + m1*(bx1-ax2);
		// check if we catch y-interval
		if (y > std::max(by1,by2) || y < std::min(by1,by2) )
			return false;

		if (swapped)	p.set(y, bx1);
		else			p.set(bx1, y);
		return true;
	}

	// both lines have finite slope
	// calculate slope and intersect with y-axis for both lines
	double m1 = dy1/dx1;
	double m2 = dy2/dx2;

	// handle case where both lines are parallel (same slope)
	if (std::fabs(m1 - m2) < EPSILON) {
		// if both lines have same intersection with y axis, they may intersect

		double b1 = ay1 - m1*ax1;
		double b2 = by1 - m2*bx1;
		if (std::fabs(b1 -b2) > EPSILON) {
			// lines are parallel but distant
			return false;
		}
		// determine scaling factors for line b in line a
		// equation of line: y(x) = b1 + scale*m1*x
		double scale_a1 = (ay1 - b1)/(ax1*m1);
		double scale_a2 = (ay2 - b1)/(ax1*m1);
		double scale_b1 = (by1 - b1)/(ax1*m1);
		double scale_b2 = (by2 - b1)/(ax1*m1);
		// both scale factors must be <= 1
		if (scale_b1 - EPSILON < scale_a1 && scale_b2 - EPSILON < scale_a1) return false;
		if (scale_b1 + EPSILON > scale_a2 && scale_b2 + EPSILON > scale_a2) return false;
		// both lines overlap
		return true;
	}

	// regular case, some arbitrary lines
	// Now, lines are neither vertical (m1,m2,b1,b2 are all finite) nor have the same slope.
	// So now there either is an actual intersection _point_ or not.
	// Do a straight-forward intersection calculation.

	double d=(ax2-ax1)*(by2-by1)-(ay2-ay1)*(bx2-bx1);

	// test, if lines intersect, factor AB is 0..1
	double AB=((ay1-by1)*(bx2-bx1)-(ax1-bx1)*(by2-by1))/d;

	// allow equality to get crossing through points
	if (AB>=0.0 - EPSILON && AB<=1.0 + EPSILON) {
		double CD=((ay1-by1)*(ax2-ax1)-(ax1-bx1)*(ay2-ay1))/d;
		if (CD>=0.0 - EPSILON && CD<=1.0 + EPSILON) {
			p.m_x=ax1+AB*(ax2-ax1);
			p.m_y=ay1+AB*(ay2-ay1);
			if (swapped)
				std::swap(p.m_x, p.m_y);
			return true;
		}
		return false;
	}
	return false;

}

} // namespace IBK


