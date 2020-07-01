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

#include "IBK_configuration.h"

#include <cmath>
#include <sstream>
#include <stdexcept>
#include <limits>

#include "IBK_math.h"
#include "IBK_Exception.h"
#include "IBK_FormatString.h"

namespace IBK {


#ifdef IBK_ENABLE_SAFE_MATH

const double POS_INF =	1e30;
const double NEAR_ZERO = 1e-10;
const double FUZZY_REL_TOLERANCE = 1e-12;

// Safe variant for std::pow function taking a double exponent
double f_pow(double base, double e) {
	// detect negative base error
	if (base < 0)
		throw IBK::Exception(IBK::FormatString("Negative base in power function, base = %1").arg(base), "[IBK::f_pow]");
	// detect overflow and throw exception
	if (e > 50)
		throw IBK::Exception(IBK::FormatString("Exponent too large in power function, exponent = %1").arg(e), "[IBK::f_pow]");
	return std::pow(base, e);
}

// Safe variant for std::pow function taking an integer exponent
double f_pow(double base, int e) {
	// detect negative base error
	if (base < 0)
		throw IBK::Exception(IBK::FormatString("Negative base in power function, base = %1.").arg(base), "[IBK::f_pow]");
	// detect overflow and throw exception
	if (e > 50)
		throw IBK::Exception(IBK::FormatString("Exponent too large in power function, exponent = %1.").arg(base), "[IBK::f_pow]");
	return std::pow(base, e);
}

// Safe variant for std::exp function
double f_exp(double x) {
	// TODO : add range check
	return std::exp(x);
}

// Safe variant for std::pow(10,x) function
double f_pow10(double e) {

	if (e > 50)
		throw IBK::Exception(IBK::FormatString("Exponent too large in power function, exponent = %1").arg(e), "[IBK::f_pow10]");

// fast pow10 function is not available on MINGW-GCC and APPLE MacOS 10.5 or non GNUC compilers
#if defined(__MINGW32__) || defined(__APPLE__) || !defined(__GNUC__)
	return std::pow(10,e);
#else
	return exp10(e);
#endif
}

// Safe variant for std::log function
double f_log(double x) {
	if (x<=0)
		throw IBK::Exception(IBK::FormatString("Negative or zero value in log function, x = %1").arg(x), "[IBK::f_log]");
	return std::log(x);
}

// Safe variant for std::log10 function
double f_log10(double x) {
	if (x<=0)
		throw IBK::Exception(IBK::FormatString("Negative or zero value in log function, x = %1").arg(x), "[IBK::f_log10]");
	return std::log10(x);
}

// Safe variant for std::sqrt function.
double f_sqrt(double x) {
	if (x<0)
		throw IBK::Exception(IBK::FormatString("Negative value in sqrt function, x = %1").arg(x), "[IBK::f_sqrt]");
	return std::sqrt(x);
}

#endif // IBK_ENABLE_SAFE_MATH

double scale(double x) {
#ifdef USE_COSINE
	return 0.5 - 0.5*std::cos(x*3.141592654);
#else //USE_COSINE
	double xinv = 1-x;
	// alternatively, you can also add an exponent to xinv
	return 1 + xinv*xinv*(-3 + 2*xinv);
#endif // USE_COSINE
}
// -----------------------------------------------------------------------------

double scale2(double x, double epsilon) {
	if (x <= 0)
		return 0;

	if (x >= epsilon)
		return 1;

#ifdef USE_COSINE
	return 0.5 - 0.5 * std::cos(x * 3.141592654 / epsilon);
#else //USE_COSINE
	double xinv = 1-x/epsilon;
	// alternatively, you can also add an exponent to xinv
	return 1 + xinv*xinv*(-3 + 2*xinv);
#endif // USE_COSINE
}

double error_function(double x) {
	// Implementation based on approximation,
	// see Abramowitz and Stegun, 1972, Handbook of Mathematical Functions,
	// National Bureau of Standards, Applied Mathematics Series 55, page 299, 7.1.25
	double ra = std::fabs(x);
	double sign = ra/x;
	double ex = std::exp(-1.0 * ra * ra);
	double T = 1.0 / (1.0 + 0.47047 * ra);
	double b = 1.0 - ex * (T * (0.3480242 - T * (0.0958798 - T * 0.7478556) ) );
	return sign * b;
}
// -----------------------------------------------------------------------------


void min_max_values(const std::vector<double> & vec, double & minVal, double & maxVal) {
	minVal = std::numeric_limits<double>::max();
	maxVal = 0;
	for (unsigned int i=0; i<vec.size(); ++i) {
		if (minVal > vec[i])
			minVal = vec[i];
		if (maxVal < vec[i])
			maxVal = vec[i];
	}
}
// -----------------------------------------------------------------------------


} // namespace IBK
