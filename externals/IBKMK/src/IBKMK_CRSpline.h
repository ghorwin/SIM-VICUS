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

#ifndef IBKMK_CRSplineH
#define IBKMK_CRSplineH

#include <vector>
#include <set>
#include <iosfwd>
#include <string>
#include <iterator>

#include "IBK_Exception.h"
#include "IBK_LinearSpline.h"

namespace IBKMK {

/*! This class encapsulates two data vectors containing a lookup table and Cathill-Rom
	interpolation functionality.

	The class CRSpline can be thought of as a container of a tabulated function
	(containing the data points of that function in separate vectors m_x and m_y).
	In addition to the reading and writing functionality of such a container it
	provides the functionality to linearly interpolate between two data points using
	the member function value(). In order to do that efficiently the slopes between
	the points will be precalculated.

	Use any of the setValue() functions to alter the data in the linear spline. Before
	using the linear spline for calculation, call makeSpline(). Only a successful call
	to makeSpline() will allow use of the functions value(), nonInterpolatedValue(),
	slope() and slopes().
	
	\todo Merge this code into LinearSpline once the interpolation works.
*/
class CRSpline : public IBK::LinearSpline {
public:
	/*! Default constructor, creates an empty spline. */
	CRSpline();
	/*! Empties the spline. */
	virtual void clear();
	/*! This generates the slopes-vector and updates the cached value. */
	virtual bool makeSpline(std::string & errMsg);
	/*! Swaps the content of a linear spline with another linear spline quickly */
	virtual void swap(CRSpline& spl);
	/*! Returns an interpolated value y at a given point x.
		If x is outside the range of x value in the spline the first or
		last y value is returned respectively.
	*/
	virtual double value(double x) const;

private:
	struct polyPoints {
		//double a0;
		double a1;
		double a2;
		double a3;
	};

	/*! The pre-calculated polygon interpolation points (generated). */
	std::vector<polyPoints> m_a;
};

}  // namespace IBKMK


/*! \file IBKMK_CRSpline.h
	\brief Contains the declaration of the class CRSpline.
*/

#endif // IBKMK_CRSplineH
