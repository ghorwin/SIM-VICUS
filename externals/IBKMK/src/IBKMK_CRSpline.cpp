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

#include "IBK_configuration.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <cmath>

#include "IBKMK_CRSpline.h"
#include "IBK_algorithm.h"
#include "IBK_assert.h"
#include "IBK_Exception.h"
#include "IBK_ScalarFunction.h"

using namespace std;

namespace IBKMK {

CRSpline::CRSpline() {
}

void CRSpline::clear() {
	IBK::LinearSpline::clear();
	m_a.clear();
}

void CRSpline::swap(CRSpline& spl) {
	IBK::LinearSpline::swap(spl);
	m_a.swap(spl.m_a);
}

double CRSpline::value(double xVal) const {
	if (!valid())
		throw IBK::Exception("Spline not properly initialized. Call makeSpline() first!","[CRSpline::value]");
	if (size() == 1)
		return y()[0];
	// x value larger than largest x value?
	if (xVal > x().back())
		return y().back();
	// we use lower bound to find the correct interval
	std::vector<double>::const_iterator it = std::lower_bound(x().begin(), x().end(), xVal);
	if (it == x().begin())
		return y().front();
	// get the index of the lower limit of the current interval
	unsigned int i = static_cast<unsigned int>(std::distance(x().begin(), it) - 1);
	// evaluate polynomial
	double t = (xVal - x()[i])/(x()[i+1] - x()[i]);
	return y()[i] + t*(m_a[i].a1 + t*(m_a[i].a2 + t*m_a[i].a3));
}

bool CRSpline::makeSpline(std::string & errMsg) {
	IBK::LinearSpline::makeSpline(errMsg);
	// either if values are invalid or if we have the special case of n=1, we return
	if (!valid() || size() == 1) {
		return valid();
	}

	// compute polynomial points
	m_a.resize(size()-1);

	// loop over all intervals and compute slopes
	for (unsigned int i=0; i<m_a.size(); ++i) {
		// interval i: between points i and i+1
		double ydash_i;
		double ydash_i1;
		double ydash_i2;
		double dx_i;
		double dx_i1;
		double dx_i2;
		double dy_i;
		double dy_i1;
		double dy_i2;

		// current interval
		ydash_i1 = slopes()[i];
		dx_i1 = x()[i+1] - x()[i];
		dy_i1 = y()[i+1] - y()[i];

		// special cases
		if (i==0) {
			// left boundary, slope is zero
			ydash_i = 0;
			dx_i = dx_i1;
			dy_i = dy_i1;
		}
		else {
			ydash_i = slopes()[i-1];
			dx_i = x()[i] - x()[i-1];
			dy_i = y()[i] - y()[i-1];
		}
		if (i==m_a.size()-1) {
			// right boundary, slope is zero
			ydash_i2 = 0;
			dx_i2 = dx_i1;
			dy_i2 = dy_i1;
		}
		else {
			ydash_i2 = slopes()[i+1];
			dx_i2 = x()[i+2] - x()[i+1];
			dy_i2 = y()[i+2] - y()[i+1];
		}
		// compute mean slopes
		double dist_i = std::sqrt(dx_i*dx_i + dy_i*dy_i);
		double dist_i1 = std::sqrt(dx_i1*dx_i1 + dy_i1*dy_i1);
		double dist_i2 = std::sqrt(dx_i2*dx_i2 + dy_i2*dy_i2);
#ifdef X_DIST
		double ydash_j = (dx_i*ydash_i + dx_i1*ydash_i1)/(dx_i + dx_i1);
		double ydash_j1 = (dx_i1*ydash_i1 + dx_i2*ydash_i2)/(dx_i1 + dx_i2);
#else // X_DIST
		double ydash_j = (dist_i*ydash_i + dist_i1*ydash_i1)/(dist_i + dist_i1);
		double ydash_j1 = (dist_i1*ydash_i1 + dist_i2*ydash_i2)/(dist_i1 + dist_i2);
#endif // X_DIST
		// now compute coefficients
		//m_a[i].a0 = m_y[i];
		m_a[i].a1 = ydash_j;
		m_a[i].a2 = -3*y()[i] + 3*y()[i+1] - 2*ydash_j - ydash_j1;
		m_a[i].a3 =  2*y()[i] - 2*y()[i+1] +   ydash_j + ydash_j1;
	}

	return true;
}



}	// namespace IBKMK

