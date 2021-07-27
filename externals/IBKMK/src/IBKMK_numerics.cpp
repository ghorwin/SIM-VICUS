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

#include <cmath>
#include <stdexcept>
#include <limits>
#include <utility>
#include <iomanip>
#include <iostream>

#include <IBK_configuration.h>
#include <IBK_FormatString.h>
#include <IBK_Exception.h>

#include "IBKMK_numerics.h"

namespace IBKMK {



// ROMBERG-Integration
// (algorithm found on http://www.voidware.com/rombint.htm)
//
// Approximates the numeric integral of `f' between `a' and `b' subject
// to a given `eps'ilon of error.
//
// Use Romberg method with refinement substitution, x = (3u-u^3)/2 which
// prevents endpoint evaluation and causes non-uniform sampling.
//
// max samples 2^(ROMB_MAX)+1
//
double romberg_integral(const IBK::ScalarFunction &f, double a, double b, double eps) {
	const int ROMB_MAX = 20;

	int n, i=0;
	double h, s;

	double c[ROMB_MAX+1];
	int lim;
	double p;
	double t, u;
	double l;

	h = 2;
	c[0] = 0;
	l = 0;
	lim = 1;
	b -= a;
	for (n = 0; n < ROMB_MAX; ++n) {
		p = h/2 - 1;
		s = 0.0;
		for (i = 0; i < lim; i++) {
			t = 1-p*p;
			u = p + t*p/2;
			u = (u*b + b)/2 + a;
			s = s + t*f(u);
			p += h;
		}

		p = 4;
		t = c[0];
		c[0] = (c[0] + h*s)/2.0;
		for(i = 0; i <= n; i++) {
			u = c[i+1];
			c[i+1] = (p*c[i] - t)/(p-1);
			t = u;
			p *= 4;
		}

		if (std::fabs(c[i] - l) < eps * std::fabs(c[i]) * 16) break;
		l = c[i];
		lim <<= 1;
		h /= 2.0;
	}
	return c[i]*b*3/4;
}
// ----------------------------------------------------------------------------

// The bi-section root finding algorithm - simple but slowest.
double bisection(const IBK::ScalarFunction& f, double a, double  b, unsigned int& iterations, double eps) {
	double xm = a, f_m;
	double f_1 = f(a);
	if (f_1 == 0) return a;
	while (--iterations) {
		xm = (a+b)/2;
		f_m = f(xm);
		if (std::fabs(f_m) < eps)  return xm;
		if ((f_m<0 && f_1>0) || (f_m>0 && f_1<0))  b = xm;
		else                                       a = xm;
	}
	return xm;
}
// ----------------------------------------------------------------------------

// The regula falsi root finding algorithm - faster, but still not the fastest.
double regula_falsi(const IBK::ScalarFunction& f, double a, double b, unsigned int& iterations, double eps) {
	double xm = a, f_1, f_2, f_m;
	while (--iterations) {
		f_1 = f(a);
		f_2 = f(b);
		if (f_1==f_2) throw IBK::Exception("f(a)==f(b)!","[regula_falsi]");
		xm = b-f_2*(b-a)/(f_2-f_1);
		f_m = f(xm);
		if (std::fabs(f_m) < eps) return xm;
		if ((f_m<0 && f_1>0) || (f_m>0 && f_1<0))  b = xm;
		else                                       a = xm;
	}
	return xm;
}
// ----------------------------------------------------------------------------

// Newtons root finding algorithm - fastest, but requires the derivative.
double newton_root(const IBK::ScalarFunction& f, IBK::ScalarFunction& df, double a, unsigned int& iterations, double eps) {
	double df_1, b = a, f_2;
	while (--iterations) {
		df_1 = df(a);
		if (df_1==0) throw IBK::Exception("df(a)==0!","[newton_root]");
		b = a-f(a)/df_1;
		f_2 = f(b);
		if (std::fabs(f_2) < eps) return b;
		a = b;
	}
	return b;
}
// -----------------------------------------------------------------------------

double secant_root(const IBK::ScalarFunction& fn, double & x, unsigned int & iters,
	double relTol, double absTol, unsigned int maxIters)
{
	//std::cout << std::setprecision(3);
	// compute xold based on moderate relative and absolute tolerances
	double xm = x + x*relTol + absTol;
	double Fm = fn(xm);

#ifdef SECANT_PRINT_STATISTICS
	const unsigned int FIELD_WIDTH = 15;
	std::cout << std::setw(6) << "iter"
			  << std::setw(FIELD_WIDTH) << "x"
			  << std::setw(FIELD_WIDTH) << "F(X)"
			  << std::setw(FIELD_WIDTH) << "dx"
			  << std::setw(FIELD_WIDTH) << "lg(F)"
			  << std::endl;
#endif // SECANT_PRINT_STATISTICS

	double xm1; // holds x_{m-1}
	double Fm1; // holds F_{m-1}
	iters = 0;
	// iterate as long as we have iterations left
	while (++iters < maxIters) {
		// store current iterative solution
		xm1 = xm;
		xm = x;
		Fm1 = Fm;
		Fm = fn(xm); // evaluate function at current xm
		// compute new x
		x = xm - Fm/(Fm-Fm1)*(xm-xm1);

		// evaluate convergence criterion
		// epsilon is a value that is 'small' compared to x
		double epsilon = relTol*std::fabs(x) + absTol;
		// compute iterative step size as difference of current iterate versus last iterate
		double dx = x - xm;
		double dx_normalized = dx/epsilon;

		// print statistics
#ifdef SECANT_PRINT_STATISTICS
		std::cout
			<< std::setw(6) << std::right << iters
			<< std::setw(FIELD_WIDTH) << std::right << xm
			<< std::setw(FIELD_WIDTH) << std::right << Fm // still of old iteration
			<< std::setw(FIELD_WIDTH) << std::right << dx
			<< std::setw(FIELD_WIDTH) << std::right << std::log10(std::fabs(Fm))
			<< std::endl;
#endif // SECANT_PRINT_STATISTICS

		// compare dx_normalized with 1, dx_normalized might be negative,
		// therefore we square both sides and 1*1 is still 1.
		if (dx_normalized*dx_normalized < 1) {
			return x; // success
		}
	}
	return x; // not converged
}


/*

  FIXME : this code is not standard C++

#ifdef __BORLANDC__

double InverseFunction::operator() (double y) const {
	unsigned int iterations = 200;
	InverseAdapter f_1(m_f, y);
	double result = IBK::bisection(f_1, m_a, m_b, iterations, m_eps);
	if (iterations != 0) return result;
	iterations = 200;
	result = IBK::bisection(f_1, m_a, m_b, iterations, m_eps*10);
	if (iterations != 0) return result;
	iterations = 2000;
	IBK::bisection(f_1, m_a, m_b, iterations, m_eps*100);
	throw std::IBK::Exception( (FormatString("[IBK::InverseFunction]  "
		"Could not find function value % in given search range %...%!")
		% y % m_a % m_b).str());
}
// -----------------------------------------------------------------------------
#endif // __BORLANDC__

*/


}  // namespace IBKMK

