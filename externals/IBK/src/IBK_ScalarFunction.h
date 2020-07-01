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

#ifndef IBK_ScalarFunctionH
#define IBK_ScalarFunctionH

#include <functional>
#include <cmath>

namespace IBK {

/*! Abstract base class for the function objects taking one double as argument.
	\todo This class is currently only used by LinearSpline and by some examples of the IBKMK library. Since the
	derivative functionality is not needed by LinearSpline and only by minimization routines, this may be moved to
	the IBKMK library.
*/
class ScalarFunction : public std::unary_function<double, double> {
public:

	ScalarFunction() {}

	/*! Virtual destructor to ensure destruction of data members of derived classes.*/
	virtual ~ScalarFunction() {}

	/*! Actual implementation of the function encapsulated in this function object.
		You have to reimplement this function in your derived class!
	*/
	virtual double operator() (double x) const = 0;

	/*! Derivative of this function, calculated by using a difference quotient.
		Reimplement this function in your derived class if you know the derivative better!
	*/
	virtual double df(double x) const {
		double f1 = operator()(x);
		double h = 1e-8 * std::fabs(x);
		if (h == 0)
			h = 1e-8;
		double xh = x - h;
		h = xh - x;
		double f2 = operator()(xh);
		return (f2-f1)/h;
	}
};

}  // namespace IBK

/*! \file IBK_ScalarFunction.h
	\brief Contains declaration of class ScalarFunction.
*/

#endif // IBK_ScalarFunctionH
