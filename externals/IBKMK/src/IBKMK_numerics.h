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

#ifndef IBKMK_numericsH
#define IBKMK_numericsH

#include <functional>
#include <IBK_ScalarFunction.h>
#include <IBK_LinearSpline.h>

namespace IBKMK {

/*! Generic linear interpolation function taking a sorting criterion for the x values. */
template <typename T, class Compare>
T interpolate(const T * x_array, const T * y_array, unsigned int n, T x, Compare comp) {
	unsigned int left = 0;
	unsigned int m = n / 2;
	while (m!=left) {
		if (comp(x, x_array[m]))    n = m;      // adjust right border
		else                        left = m;   // adjust left border
		m = (left + n) / 2;
	}
	return y_array[m] + (y_array[m+1]-y_array[m])/(x_array[m+1]-x_array[m])*(x-x_array[m]);
}

/*! Generic linear interpolation function (you don't have to specify how the x values are ordered). */
template <typename T>
T interpolate(const T * x_array, const T * y_array, unsigned int n, T x) {
	if (x_array[0] < x_array[1])
		return interpolate<T>(x_array, y_array, n, x, std::less<T>());
	else
		return interpolate<T>(x_array, y_array, n, x, std::greater<T>());
}

/*! Generic linear interpolation function taking a sorting criterion for the x values. */
template <typename T, class Compare>
T interpolate(const T * x_array, const T * y_array, const T * slopes, unsigned int n, T x, Compare comp) {
	unsigned int left = 0;
	unsigned int m = n / 2;
	while (m!=left) {
		if (comp(x, x_array[m]))    n = m;      // adjust right border
		else                        left = m;   // adjust left border
		m = (left + n) / 2;
	}
	return y_array[m] + slopes[m]*(x-x_array[m]);
}

/*! Generic linear interpolation function (you don't have to specify how the x values are ordered). */
template <typename T>
T interpolate(const T * x_array, const T * y_array, const T * slopes, unsigned int n, T x) {
	if (x_array[0] < x_array[1])
		return interpolate<T>(x_array, y_array, slopes, n, x, std::less<T>());
	else
		return interpolate<T>(x_array, y_array, slopes, n, x, std::greater<T>());
}

/*! Performs a numeric integration using the Romberg algorithm.
	\param f 	The function object encapsulating the function.
	\param a	The lower bound of the integration range.
	\param b	The upper bound of the integration range.
	\param eps  (optional) The allowed relative tolerance.
*/
double romberg_integral(const IBK::ScalarFunction &f, double a, double b, double eps);

/*! The bi-section root finding algorithm - simple but slowest.
	Finds x so that f(x) = 0.
	\param f 			The function object encapsulating the function f(x).
	\param a			The lower bound of the search range.
	\param b			The upper bound of the search range.
	\param iterations	The maximum number of iterations to be performed. Must be a
						reference variable. After algorithm has finished it contains
						the number of iterations left.
						If iterations==0 the returned solution is outside the
						tolerance margin.
	\param eps			(optional) The allowed relative tolerance.
*/
double bisection(const IBK::ScalarFunction& f, double a, double  b, unsigned int& iterations, double eps);

/*! The regula falsi root finding algorithm - faster than bisection, but still not the fastest.
	Finds x so that f(x) = 0.
	\param f 			The function object encapsulating the function f(x).
	\param a			The lower bound of the search range.
	\param b			The upper bound of the search range.
	\param iterations	The maximum number of iterations to be performed. Must be a
						reference variable. After algorithm has finished it contains
						the number of iterations left.
						If iterations==0 the returned solution is outside the
						tolerance margin.
	\param eps			(optional) The allowed relative tolerance.
*/
double regula_falsi(const IBK::ScalarFunction& f, double a, double b, unsigned int& iterations, double eps);

/*! Newtons root finding algorithm - fastest, but requires the derivative.
	Finds x so that f(x) = 0.
	\param f 			The function object encapsulating the function f(x).
	\param df 			The function object encapsulating df/dx, the derivative of the function f(x).
	\param a			The starting value for the search (should be already near the solution).
	\param iterations	The maximum number of iterations to be performed. Must be a
						reference variable. After algorithm has finished it contains
						the number of iterations left.
						If iterations==0 the returned solution is outside the
						tolerance margin.
	\param eps			(optional) The allowed relative tolerance.
*/
double newton_root(const IBK::ScalarFunction& f, IBK::ScalarFunction& df, double a, unsigned int& iterations, double eps);

/*! Secant method. */
double secant_root(const IBK::ScalarFunction& f, double & x, unsigned int & iters, double relTol, double absTol, unsigned int maxIters = 100);

/*! This is an adapter class, that calculates g(x, y) = y - f(x), whereas f(x) can
	be an arbitrary function encapsulated in a function object.
*/
class InverseAdapter : public IBK::ScalarFunction {
  public:
	/*! Constructor, creates the InverseAdapter function object. */
	InverseAdapter(const IBK::ScalarFunction & f, double y) : m_y(y), m_f(&f) {}
	/*! Calculates the function g(x, y) = y - f(x). */
	double operator()(double x) const { return m_y - (*m_f)(x); }

	double 				m_y;	///< The value y
	const IBK::ScalarFunction 	*m_f;	///< The function f(x)
};

class SplineAdapter : public IBK::ScalarFunction {
public:
	SplineAdapter(const IBK::LinearSpline& spl) : m_spl(spl) {}
	double operator() (double x) const { return m_spl.value(x); }

	IBK::LinearSpline m_spl;
};

} // namespace IBKMK

#endif // IBKMK_numericsH
