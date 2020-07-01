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

#ifndef IBK_mathH
#define IBK_mathH

#include "IBK_configuration.h"
#include "IBK_assert.h"

#include <string>
#include <cmath>
#include <vector>
#include <cstring>
#include <cstdio>

#ifdef _MSC_VER
	typedef __int64 int64_t;
	typedef unsigned __int64 uint64_t;
	typedef unsigned __int32 uint32_t;
	typedef unsigned __int16 uint16_t;
#else
	#include <stdint.h>
#endif

namespace IBK {

extern const double POS_INF;							///< Positive infinity limit.
extern const double NEAR_ZERO;							///< Clipping constant, used for fuzzy comparison operations.
extern const double FUZZY_REL_TOLERANCE;				///< Relative tolerance to be used in fuzzy comparisons.


#ifdef IBK_ENABLE_SAFE_MATH

/*! Safe variant for std::pow function taking a double exponent. */
double f_pow(double base, double e);

/*! Safe variant for std::pow function taking an integer exponent. */
double f_pow(double base, int e);

/*! Safe variant for std::pow(10,x) function. */
double f_pow10(double e);

/*! Safe variant for std::exp function. */
double f_exp(double x);

/*! Safe variant for std::log function. */
double f_log(double x);

/*! Safe variant for std::log10 function. */
double f_log10(double x);

/*! Safe variant for std::sqrt function. */
double f_sqrt(double x);

#else // IBK_ENABLE_SAFE_MATH

/*! Power function wrapper for real exponent. */
inline double f_pow(double base, double e) 			{ return std::pow(base,e); }
/*! Power function wrapper for integer exponent. */
inline double f_pow(double base, int e) 			{ return std::pow(base,e); }
/*! Power function wrapper for base 10. */
inline double f_pow10(double e)						{
#if defined(__MINGW32__) || defined(__APPLE__) || !defined (__GNUC__)
	return std::pow(10,e);
#else
	return pow10(e);
#endif
}

/*! Exponential function wrapper. */
inline double f_exp(double x)						{ return std::exp(x); }
/*! Logarithm function wrapper. */
inline double f_log(double x)						{ return std::log(x); }
/*! Logarithm to base 10 function wrapper. */
inline double f_log10(double x)						{ return std::log10(x); }
/*! Square-root function wrapper. */
inline double f_sqrt(double x)						{ return std::sqrt(x); }

#endif // IBK_ENABLE_SAFE_MATH

/*! Comparison operator Greater-Than or Equal >= which allows for small relative rounding errors. */
inline bool f_fuzzyGTEQ(double left, double right) {
	// left is >= right when we add some very small value to left and this is larger than right
	return (left*(1 + FUZZY_REL_TOLERANCE) + FUZZY_REL_TOLERANCE > right);
}

/*! Comparison operator Less-Than or Equal <= which allows for small relative rounding errors. */
inline bool f_fuzzyLTEQ(double left, double right) {
	// left is <= right when we add some very small value to right and this is larger than left
	return (left < right*(1 + FUZZY_REL_TOLERANCE) + FUZZY_REL_TOLERANCE);
}

/*! Comparison operator == which allows for small relative rounding errors. */
inline bool f_fuzzyEQ(double left, double right) {
	// left is == right when we add some very small value to left and this is larger than right
	// and we can do the same for right
	return f_fuzzyGTEQ(left, right) && f_fuzzyLTEQ(left, right);
}

/*! Moved function to the right place.
	Right now epsilon is declared multiple times, just use FUZZY_REL_TOLERANCE, which is defined here!
	\todo rewrite code to use preexisting functionf_fuzzyGT etc.	*/
inline bool simpleFuzzyEq(const double & v1, const double & v2, const double & epsilon) {
	const double diff = v1 - v2;
	return diff < epsilon && diff > -epsilon;
}

/*! Moved function to the right place.
	Right now epsilon is declared multiple times, just use FUZZY_REL_TOLERANCE, which is defined here!
	\todo rewrite code to use preexisting functionf_fuzzyGT etc.	*/
inline bool fuzzyBetween(const double & v, const double & vMin, const double & vMax, const double & epsilon) {
	return v > vMin && v < vMax && !simpleFuzzyEq(v, vMin, epsilon) && !simpleFuzzyEq(v, vMax, epsilon);
}

/*! Moved function to the right place.
	Right now epsilon is declared multiple times, just use FUZZY_REL_TOLERANCE, which is defined here!
	\todo rewrite code to use preexisting functionf_fuzzyGT etc.	*/
inline bool betweenOrEqual(const double & v, const double & vMin, const double & vMax, const double & epsilon) {
	return (v > vMin && v < vMax) || simpleFuzzyEq(v, vMin, epsilon) || simpleFuzzyEq(v, vMax, epsilon);
}


/// Utility function that can be used to set all elements of a Vector to zero very quickly.
inline void set_zero(std::vector<double>& v) { std::memset(&v[0], 0, sizeof(double)*v.size()); }

/*! Power function version for integer exponents.

	Use this function to evaluate integer powers of double values, like
	in the following example:
	\code
	// calculate the fifth power of a double value
	double a = 14.56;
	double result = f_powN<5>(a);
	\endcode
	Faster implementation than f_pow(a, 5).
*/
template<unsigned int exp>
double f_powN(double base) {
	double prod(1.0);
	for (unsigned int i=1; i <= exp; i++)
		prod *= base;
	return(prod);
}

/*! Function for efficient calculation of x^4. */
inline double f_pow4(double x) { double x2(x); x2*=x2; x2*=x2; return x2; }

/*! Tests if a is equal to b including a certain range for potential rounding errors. */
inline bool near_equal(double a, double b) {
	return (a + NEAR_ZERO >= b  &&  a <= b + NEAR_ZERO);
}

/*! Tests if a is equal to b including a certain range for potential rounding errors. */
inline bool near_zero(double a) {
	return (a  >= -NEAR_ZERO  &&  a <= NEAR_ZERO);
}

/*! Meta template struct for calulating 10 power N with positiv N.
	\code
	double p10_to_power_8 = pow10Pos<8>();
	\endcode
	\note This template is used in function nearly_equation<int>(x,y).
*/
template <int N> struct pow10Pos {
	/*! Power constant to speed up power calculation. */
	static const int pow = 10 * pow10Pos<N-1>::pow;
};

/*! Meta template struct spezialisation for N=1 for calulating 10 power N with positiv N.*/
template <> struct pow10Pos<1> {
	/*! Power constant to speed up power calculation, specialization for N=1. */
	static const int pow = 10;
};

/*! Meta template struct spezialisation for N=0 for calulating 10 power N with positiv N.*/
template <> struct pow10Pos<0> {
	/*! Power constant to speed up power calculation, specialization for N=0. */
	static const int pow = 1;
};

/*! Compares equality of a and b while using a certain number of digits. */
template<int digits>
bool nearly_equal(double x, double y) {
	if( x == y ) //-V550
		return true;

	const double eps = 1.0 / pow10Pos<digits>::pow;
	return (x + eps >= y  &&  x <= y + eps);
}


/*! Tests if a is less then b including a certain range for potential rounding errors. */
inline bool near_le(double a, double b)     { return !near_equal(a, b) && a < b; }
/*! Tests if a is less or equal to b including a certain range for potential rounding errors. */
inline bool near_le_eq(double a, double b)  { return (a < b || a - NEAR_ZERO < b); }
/*! Tests if a is greater then b including a certain range for potential rounding errors. */
inline bool near_gr(double a, double b)     { return (a > b && !near_equal(a, b)); }
/*! Tests if a is greater or equal to b including a certain range for potential rounding errors. */
inline bool near_gr_eq(double a, double b)  { return (a > b || a + NEAR_ZERO > b); }

/*! The unary function abs returns the absolute value of 'value'
	(which only makes sense with numerical data types).

	\note This function object is useful for transforming algorithms. Otherwise std::fabs() can
			be also used or in time-critical code an own local ABS macro.
*/
template <typename T>
struct abs {
	/*! Evaluation operator, returns absolute value of value argument. */
	T operator()(const T& value) const { return (value > 0) ? value : -value; }
};

/*! This function is a non-linear scaling/ramping function to allow for smooth transition between
	discontinuities in functions.
	The function has the following defined values:
	- scale(x=0) = 0
	- scale(x=1) = 1
	- scale(x=-1) = 1
	- dscale/dx(x=0) = 0
	- dscale/dx(x=1) = 0

	\param x Value between 0..1 (no check done!).
*/
double scale(double x);

/*! This function is a non-linear scaling function.
	The function has the following defined values:
	- scale(x<=0) = 0
	- scale(x>=epsilon) = 1
	- dscale/dx(x<=0) = 0
	- dscale/dx(x>=epsilon) = 0

	\param x Value between 0..1.
	\param epsilon Thickness of transition area
*/
double scale2(double x, double epsilon);

/*! Same as function above but without the limit on the x value argument.
	Returns 0 for x < 0, and 1 for x > 1, for 0 < x < 1 returns the same
	value as scale().
*/
inline double scale2(double x) { return scale2(x, 1.0); }


/*! Implementation of the error function, based on an approximation equation
	with epsilon = 1e-5.
*/
double error_function(double x);

/*! Determines min and max values in a vector. */
void min_max_values(const std::vector<double> & vec, double & minVal, double & maxVal);

} // namespace IBK

/*! \file IBK_math.h
	\brief Contains safe variants of standard C math functions and other utility functions.
*/

#endif // IBK_mathH
