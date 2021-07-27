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

#ifndef IBKMK_minimizationH
#define IBKMK_minimizationH

#include <cmath>
#include <algorithm>
#include <limits>

#include <IBK_ScalarFunction.h>
#include <IBK_Exception.h>

namespace IBKMK {

/*! Base class for minimization algorithms.
	This class implements the algorithm used to bracket the minimum in order to ensure
	a successful minimization.
*/
class MinimizerBase {
public:
	/*! Constructor.
		@param eps   The accuracy of the solution (don't set it to less then sqrt(numeric_limits&lt;double&rt;(epsilon)) ).
	*/
	MinimizerBase(double par_eps = 3e-8, const unsigned int par_max_iters = 1000)
		: eps(par_eps), max_iters(par_max_iters)
	{
	}

	/*! This function calculates a, b and c in such a way, that they form a valid minimization bracket.
		A valid minimization bracket is found when a < b < c and f(a) > f(b) and f(b) < f(c).
		@param f            The minimization function.
		@param a            The left side of the guessed bracket.
		@param b            Another point in the interval, where a minimum is probable.
		@return             The function does not return anything, but the values a, b, c will contain the bracket values.
	*/
	template <typename Func>
	void bracketMinimum(Func& f, double a, double b);

	///< Tolerance.
	double eps;

	///< Hold the minimum value (return value) from any of the minimization implementations.
	double xmin;
	///< Function value for the minimum value.
	double fmin;

	double ax;
	double bx;
	double cx;
	double fa;
	double fb;
	double fc;

	/*! Holds the number of remaining iterations after a minimization function returned. */
	unsigned int iterations;

	/*! Holds the default maximum of iterations to use. */
	unsigned int max_iters;

	/*! Depending on the sign of b, it will return a (if b is positive), or -a (if be is negative). */
	static inline double sign(double a, double b) {
		return b >= 0.0 ? std::fabs(a) : -std::fabs(a);
	}

	static inline double GOLDEN_SECTION() 		{ return  0.61803399; }
	static inline double GOLDEN_SECTION_1() 	{ return  1.61803399; }
	static inline double GOLDEN_SECTION_COMP() 	{ return 1.0 - GOLDEN_SECTION(); } // = 0.3819...
	static inline double GOLDEN_LIMIT() 		{ return 100.0; }
	static inline double TINY() 				{ return 1e-20; }
};

/*! Implementation of the Golden Section Search. */
class MinimizerGoldenSection : public MinimizerBase {
public:
	MinimizerGoldenSection(double par_eps = 3e-8, const unsigned int par_max_iters = 1000)
		: MinimizerBase(par_eps,par_max_iters)
	{
	}

	/*! This function calculates the minimum of the function f in the interval a, c.
		The function requires that the member variables ax, bx and cx form a valid minimization bracket.
		A valid minimization bracket is given when ax < bx < cx and f(ax) > f(bx) and f(bx) < f(cx).
		You can use the member function bracketMinimum() to determine the initial bracket.
		This function is re-implemented in the various minimization base classes.
		@param f	The minimization function.
		@return		Returns the minimum if found or the closest value to it.
	*/
	template <typename Func>
	double minimize(Func& f);
};

/*! Implementation of Brent's method (parabolic fit). */
class MinimizerBrent : public MinimizerBase {
public:
	MinimizerBrent(double par_eps = 3e-8, const unsigned int par_max_iters = 1000)
		: MinimizerBase(par_eps,par_max_iters)
	{
	}
	/*! This function calculates the minimum of the function f in the interval a, c.
		The function requires that the member variables ax, bx and cx form a valid minimization bracket.
		A valid minimization bracket is given when ax < bx < cx and f(ax) > f(bx) and f(bx) < f(cx).
		You can use the member function bracketMinimum() to determine the initial bracket.
		This function is re-implemented in the various minimization base classes.
		@param f	The minimization function.
		@return		Returns the minimum if found or the closest value to it.
	*/
	template <typename Func>
	double minimize(Func& f);
};

/*! Implementation of Brent's method (parabolic fit) using derivatives. */
class MinimizerBrentDerivative : public MinimizerBase {
public:
	MinimizerBrentDerivative(double par_eps = 3e-8, const unsigned int par_max_iters = 1000)
		: MinimizerBase(par_eps,par_max_iters)
	{
	}
	/*! This function calculates the minimum of the function f in the interval a, c.
		The function requires that the member variables ax, bx and cx form a valid minimization bracket.
		A valid minimization bracket is given when ax < bx < cx and f(ax) > f(bx) and f(bx) < f(cx).
		You can use the member function bracketMinimum() to determine the initial bracket.
		This function is re-implemented in the various minimization base classes.
		@param f	The minimization function, requires f() and df() to be implemented.
		@return		Returns the minimum if found or the closest value to it.
	*/
	template <typename Func>
	double minimize(Func& f);
};

template <typename T>
void MinimizerBase::bracketMinimum(T& f, double a, double b) {
	ax = a;
	bx = b;
	fa = f(ax);
	fb = f(bx);
	// switch directions so that the downhill direction is a -> b
	if (fb > fa) {
		std::swap(ax, bx);
		std::swap(fa, fb);
	}
	// if we don't have a given c, guess a first value for c
	cx = bx + GOLDEN_SECTION_1()*(bx-ax);
	fc = f(cx);
	double fu;
	// now step downhill until fa > fb && fb < fc
	while (fb > fc) {
		// compute parabolic extrapolation from a, b, c
		double r = (bx-ax)*(fb-fc);
		double q = (bx-cx)*(fb-fa);
		double u = bx - ( (bx-cx)*q - (bx-ax)*r)/
			(2.0*sign(std::max<double>(std::fabs(q-r), TINY()), q-r) );
		// calculate a limit that we will never overstep
		double ulim = bx + GOLDEN_LIMIT()*(cx-bx);
		// test various possibilities
		if ((bx-u)*(u-cx) > 0) {
			// parabolic u might be between b and c, try it
			fu = f(u);
			if (fu < fc) {	// got a minimum between b and c,
				ax = bx; // store our bracket and return
				bx = u;
				fa = fb;
				fb = fu;
				return;
			}
			else if (fu > fb) { // got a minimum between a and u
				cx = u;
				fc = fu;
				return;
			}
			// parabolic fit didn't work, use standard magnification
			u = cx + GOLDEN_SECTION_1()*(cx-bx);
			fu = f(u);
		}
		else if ((cx-u)*(u-ulim) > 0) {
			// parabolic fit might be between u and its limit ulim
			fu = f(u);
			if (fu < fc) {
				// it is, so set new bracket to b, u, c
				bx=cx; cx=u; u=u + GOLDEN_SECTION_1()*(u-cx);
				fb=fc; fc=fu; fu=f(u);
			}
		}
		else if ((u-ulim)*(ulim-cx) >= 0) {
			// limit u to maximum allowed value
			u = ulim;
			fu = f(u);
		}
		else {
			// parabolic fit didn't work, use standard magnification
			u = cx + GOLDEN_SECTION_1()*(cx-bx);
			fu = f(u);
		}
		// move bracket
		ax=bx; bx=cx; cx=u;
		fa=fb; fb=fc; fc=fu;
	}
	if (ax > cx) {
		std::swap(ax, cx);
		std::swap(fa, fc);
	}
}
// -----------------------------------------------------------------------------

template <typename T>
double MinimizerGoldenSection::minimize(T& f) {
	// transfer maximum number of iterations
	iterations = max_iters;
	// required: ax < bx < cx
	if (ax > bx || bx > cx)
		throw IBK::Exception("Invalid input. Required: a < b < c!", "[MinimizerGoldenSection::minimize]");

	// also required: f(a) > f(b) && f(b) < f(c)
	if (f(ax) < f(bx) || f(bx) > f(cx))
		throw IBK::Exception("Invalid input. Required: f(a) > f(b) && f(b) < f(c)!", "[MinimizerGoldenSection::minimize]");

	// golden section search algorithm, based on Numerical Recipes in C code
	// four points are considered i=0...3
	double x0 = ax; // x0 is the left boundary of the bracket
	double x3 = cx; // x3 is the right boundary of the bracket
	double x1, x2;
	// make x0-x1 the smaller segment
	if (std::fabs(cx-bx) > std::fabs(bx-ax)) {
		x1 = bx;
		x2 = bx + GOLDEN_SECTION_COMP()*(cx-bx);
	}
	else {
		x1 = bx - GOLDEN_SECTION_COMP()*(bx-ax);
		x2 = bx;
	}
	// initial function evaluations
	double f1 = f(x1);
	double f2 = f(x2);
	// now loop until we have found the minimum
	while (std::fabs(x3-x0) > eps*(std::fabs(x1)+std::fabs(x2))) {
		// one of the possible outcomes
		if (f2 < f1) {
			// shift the bracket to the left
			x0 = x1; x1 = x2; x2 = GOLDEN_SECTION()*x1 + GOLDEN_SECTION_COMP()*x3;
			// shift the function evaluations
			f1 = f2; f2 = f(x2);
		}
		else {
			// shift bracket to the right
			x3 = x2; x2 = x1; x1 = GOLDEN_SECTION()*x2 + GOLDEN_SECTION_COMP()*x0;
			// shift the function evaluations
			f2 = f1; f1 = f(x1);
		}
		// check that our iteration limit wasn't exceeded
		if (--iterations == 0)  {
			xmin = 0.5*(x1+x2);
			fmin = f(xmin);
			return xmin;
		}
	}
	if (f1 < f2)	{
		fmin = f1;
		return xmin = x1;
	}
	else {
		fmin = f2;
		return xmin = x2;
	}
}
// -----------------------------------------------------------------------------

template <typename T>
double MinimizerBrent::minimize(T& f) {
	const double ZEPS = std::numeric_limits<double>::epsilon()*1e-3;

	iterations = max_iters;

	// required: ax < bx < cx
	if (ax > bx || bx > cx)
		throw IBK::Exception("Invalid input. Required: a < b < c!", "[MinimizerBrent::minimize]");

	// also required: f(a) > f(b) && f(b) < f(c)
	if (fa < fb || fb > fc)
		throw IBK::Exception("Invalid input. Required: f(a) > f(b) && f(b) < f(c)!", "[MinimizerBrent::minimize]");

	double a = ax;
	double b = cx;

	double d = 0;
	double e = 0; // distance moved in the step before last, used to check convergence rate

	double x,v,w,u;
	x = v = w = bx;

	double fx, fv, fw, fu;
	fx = fv = fw = f(bx);

	double r,p,q,etemp;

	// loop until converged or out of iterations
	while (--iterations) {
		double xm = (a+b)*0.5;					// middle of interval
		double tol1 = eps*std::fabs(x) + ZEPS;	// some small distance
		double tol2 = 2.0*tol1;					// times two
		// check if our interval is small enough already
		if (std::fabs(x-xm) <= (tol2 - 0.5*(b-a))) {
			fmin = fx;
			return xmin = x;
		}
		// still converging?
		if (std::fabs(e) > tol1) {
			// compose parabolic fit
			r = (x-w)*(fx-fv);
			q = (x-v)*(fx-fw);
			// compute numerator
			p = (x-v)*q - (x-w)*r;
			// compute denominator
			q = 2.0*(q-r);
			if (q > 0.0)
				p = -p;
			else
				q = -q; // make q positive, this is faster than std::fabs(q)
			// store current e for comparison with the overnext step
			etemp = e;
			e = d;
			// check if the parabolic fit is acceptable
			if (std::fabs(p) >= std::fabs(0.5*q*etemp) ||	/* denominator near zero (collinear points) */
				p <= q*(a-x) ||	/* would take us out of bounds */
				p >= q*(b-x))	/* would take us out of bounds */
			{
				// can't take parabolic step, need to use plain old Golden section
				// determine direction we're moving
				if (x >= xm)	e = a-x;	// to the left (negative)
				else			e = b-x;	// to the right (positive)
				// estimate new distance from current x
				d = GOLDEN_SECTION_COMP()*e;
			}
			else {
				// yeah, let's go parabolic
				d = p/q;
				u = x+d; // new x
				// check if we moved too close to one of the interval boundaries
				if (u-a < tol2 || b-u < tol2) {
					// reject parabolic estimate and move from the center into the larger interval
					d = sign(tol1, xm-x);
				}
			}
		}
		else {
			// Fall back to Golden Section
			// determine direction we're moving
			if (x >= xm)	e = a-x;	// to the left (negative)
			else			e = b-x;	// to the right (positive)
			// estimate new distance from current x
			d = GOLDEN_SECTION_COMP()*e;
		}
		// if the distance is estimated too small, we move at least tol1 distance away from the current point
		if (std::fabs(d) >= tol1)		u = x + d;
		else							u = x + sign(tol1, d);
		// evaluate the function at the new point
		fu = f(u);
		// use larger of the values for our new interval, so that we always keep a bracketed minimum
		// between a and b (with x at the probable minimum position).
		if (fu < fx) {
			// adjust our interval
			if ( u >= x)		a = x;
			else				b = x;
			// some more housekeeping, to avoid unnecessary function evaluations
			v=w;   w=x;   x=u;
			fv=fw; fw=fx; fx=fu;
		}
		else {
			// adjust our interval
			if (u<x)			a = u;
			else				b = u;
			// some more housekeeping, to avoid unnecessary function evaluations
			if (fu <= fw) {
				v=w; w=u;
				fv=fw; fw=fu;
			}
			else if (fu <= fv || v ==x || v == w) {
				v=u;
				fv=fu;
			}
		}
	}
	fmin = fx;
	return xmin = x;
}
// -----------------------------------------------------------------------------

template <typename T>
double MinimizerBrentDerivative::minimize(T& f) {
	const double ZEPS = std::numeric_limits<double>::epsilon()*1e-3;

	bool ok1, ok2; // flags for proposed steps
	double d1, d2, du, dv, dw, dx;
	double fu, fv, fw, fx, old_e, tol1, tol2, u, u1, u2, v, w, x, xm;

	iterations = max_iters;

	// required: ax < bx < cx
	if (ax > bx || bx > cx)
		throw IBK::Exception("Invalid input. Required: a < b < c!", "[MinimizerBrentDerivative::minimize]");

	// also required: f(a) > f(b) && f(b) < f(c)
	if (fa < fb || fb > fc)
		throw IBK::Exception("Invalid input. Required: f(a) > f(b) && f(b) < f(c)!", "[MinimizerBrentDerivative::minimize]");

	double a = ax;
	double b = cx;

	double d = 0; // the step to take
	double e = 0;

	x=w=v=bx;
	fw=fv=fx=f(x);
	dw=dv=dx=f.df(x);
	// loop until converged or out of iterations
	while (--iterations) {
		xm = 0.5*(a+b); // go to half of interval
		tol1 = eps*fabs(x) + ZEPS;
		tol2 = 2.0*tol1;

		// interval is already small enough?
		if (fabs(x-xm) <= (tol2 - 0.5*(b-a))) {
			fmin = fx;
			return xmin = x;
		}

		if (fabs(e) > tol1) {
			// initialize offsets d2 and d1 with the double of the center of the interval
			d2 = d1 = 2.0*(b-a);
			if (dw != dx)
				d1 = (w-x)*dx/(dx-dw); // secant method to point one
			if (dv != dx)
				d2 = (v-x)*dx/(dx-dv); // and to point two
			// reject estimates that are outside of the bracket
			u1 = x + d1;
			u2 = x + d2;
			ok1 = (a-u1)*(u1-b) > 0.0 && dx*d1 <= 0.0;
			ok2 = (a-u2)*(u2-b) > 0.0 && dx*d2 <= 0.0;
			old_e = e;
			e = d;
			// check if we have an exceptable d
			if (ok1 || ok2) {
				if (ok1 && ok2) {
					// both are exceptable
					// take the point where the function value is smaller
					if (fabs(d1) < fabs(d2))	d = d1;
					else						d = d2;
				}
				else if (ok1) {
					d = d1;
				}
				else {
					d = d2;
				}
				// now that we have a new point, check if we are
				// actually better than the old one
				if (fabs(d) <= fabs(0.5*old_e)) {
					u = x + d;
					if (u-a < tol2 || b-u < tol2) {
						d = sign(tol1, xm-x);
					}
				}
				else {
					// otherwise use bisection method
					if (dx >= 0.0)	e = a-x;
					else			e = b-x;
					d = 0.5*e;
				}
			}
			else {
				// none of the points was exceptable
				// use bisection method
				if (dx >= 0.0)	e = a-x;
				else			e = b-x;
				d = 0.5*e;
			}
		}
		else {
			// not enough derivative information yet
			// use bisection method to get a new point
			if (dx >= 0.0)	e = a-x;
			else			e = b-x;
			d = 0.5*e;
		}
		// new step bigger than minimum step?
		if (fabs(d) >= tol1) {
			u = x+d;
			fu = f(u);
		}
		else {
			// take at least the minimum step further
			u = x + sign(tol1, d);
			fu = f(u);
			if (fu > fx) {
				// minimum step goes uphill, so we're done
				fmin = fx;
				return xmin = x;
			}
		}
		// evaluate derivative and do other housekeeping
		du = f.df(u);
		if (fu <= fx) {
			if (u >= x)		a = x;
			else			b = x;
			v = w; fv = fw; dv = dw;
			w = x; fw = fx; dw = dx;
			x = u; fx = fu; dx = du;
		}
		else {
			if (u < x)		a = u;
			else			b = u;
			if (fu <= fw || w == x) {
				v = w; fv = fw; dv = dw;
				w = u; fw = fu; dw = du;
			}
			else if (fu < fx || v == x || v == w) {
				v = u; fv = fu; dv = du;
			}
		}
	}
	fmin = fx;
	return xmin = x;
}


} // namespace IBKMK

#endif // IBKMK_minimizationH
