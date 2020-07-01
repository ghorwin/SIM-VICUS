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

#ifdef _MSC_VER
	typedef __int64 int64_t;
	typedef unsigned __int64 uint64_t;
	typedef unsigned __int32 uint32_t;
	typedef unsigned __int16 uint16_t;
#else
	#include <stdint.h>
#endif

#include <iostream>
#include <sstream>
#include <limits>
#include <functional>
#include <stdexcept>
#include <iomanip>
#include <iterator>
#include <ctime>


#include "IBK_LinearSpline.h"
#include "IBK_InputOutput.h"
#include "IBK_algorithm.h"
#include "IBK_assert.h"
#include "IBK_Exception.h"
#include "IBK_ScalarFunction.h"

namespace IBK {

LinearSpline::LinearSpline() :
	m_extrapolationMethod(EM_Constant),
	m_xMin(0),
	m_xMax(0),
	m_xStep(0),
	m_xOffset(0),
	m_valid(false)
{
}

unsigned int LinearSpline::size()  const {
	return m_y.size();
}


void LinearSpline::eliminateConsecutive(	const std::vector<double>& tmp_x,
											const std::vector<double>& tmp_y,
											std::vector<double>& tmp2_x,
											std::vector<double>& tmp2_y)
{
	const char * const FUNC_ID = "[LinearSpline::eliminateConsecutive]";
	// eliminate consecutive x values (stem from missing accuracy when reading the file)
	tmp2_x.clear();
	tmp2_y.clear();
	tmp2_x.push_back(tmp_x.front());
	tmp2_y.push_back(tmp_y.front());
	unsigned int skipped_values = 0;
	double last_reported_duplicate = std::numeric_limits<double>::infinity();
	for (unsigned int i=1, size = static_cast<unsigned int>(tmp_x.size()); i<size; ++i) {
		if (tmp_x[i] != tmp2_x.back()) {
			tmp2_x.push_back(tmp_x[i]);
			tmp2_y.push_back(tmp_y[i]);
		}
		else {
			if (tmp2_x.back() != last_reported_duplicate) {
				IBK::IBK_Message(FormatString("Duplicate x-value %1 found (and skipped).").arg(tmp2_x.back()), MSG_WARNING, FUNC_ID, 3);
				last_reported_duplicate = tmp2_x.back();
			}
			++skipped_values;
		}
	}
	// If the spline ends with a zero slope line sets y value to the original end y value
	if( tmp_x.back() == tmp2_x.back() && tmp_y.back() != tmp2_y.back()) {
		IBK::IBK_Message(FormatString("Duplicate x-value found at the end. End y value %1 copied to the new spline (old y value: %2.")
						 .arg(tmp_y.back()).arg(tmp2_y.back()), MSG_WARNING, FUNC_ID, 3);
		tmp2_y.back() = tmp_y.back();
	}
	if (skipped_values != 0) {
		IBK::IBK_Message(FormatString("Skipped a total of %1 values because of duplicate x-values.").arg(skipped_values), MSG_WARNING, FUNC_ID, 3);
	}
}


void LinearSpline::setValues(const std::vector<double> & xvals, const std::vector<double> & yvals) {
	const char * const FUNC_ID = "[LinearSpline::setValues]";
	if (xvals.size() != yvals.size())
		throw IBK::Exception("X and Y vector size mismatch.", FUNC_ID);
	if (xvals.empty())
		throw IBK::Exception("Input vectors are empty.", FUNC_ID);
	std::vector<double> tmp_x;
	std::vector<double> tmp_y;
	eliminateConsecutive(xvals, yvals, tmp_x, tmp_y);
	if (tmp_x.empty())
		throw IBK::Exception("Input vectors are empty.", FUNC_ID);
	setValues(tmp_x.begin(), tmp_x.end(), tmp_y.begin());
	std::string errstr;
	m_valid = makeSpline(errstr);
	if (!errstr.empty()) {
		/// \todo Check this, shouldn't this "warning" be handled as exception instead?
		IBK::IBK_Message(FormatString("Error while makeSpline in setValues. %1").arg(errstr), MSG_WARNING, FUNC_ID, 3);
	}
}


bool LinearSpline::read(const std::string& x_data, const std::string& y_data, std::string * errorMsg) {

	std::vector<double> tmp_x;
	std::vector<double> tmp_y;

	string2valueVector(x_data, tmp_x);
//	std::stringstream xstrm(x_data);
//	std::copy(std::istream_iterator<double>(xstrm), std::istream_iterator<double>(), std::back_inserter(tmp_x) );

	string2valueVector(y_data, tmp_y);
//	std::stringstream ystrm(y_data);
//	std::copy(std::istream_iterator<double>(ystrm), std::istream_iterator<double>(), std::back_inserter(tmp_y) );

	if (tmp_x.empty()) {
		if (errorMsg != NULL) *errorMsg = "Missing data to read!";
		return false;
	}
	if (tmp_x.size() != tmp_y.size()) {
		if (errorMsg != NULL) {
			std::stringstream errstrm;
			errstrm << "Number of x [" << tmp_x.size() << "] and y ["<< tmp_y.size() <<"] values differs!";
			*errorMsg = errstrm.str();
		}
		return false;
	}
	std::vector<double> tmp2_x;
	std::vector<double> tmp2_y;

	eliminateConsecutive(tmp_x, tmp_y, tmp2_x, tmp2_y);

	// now finally we have the data we need and can update our local variables
	m_x = tmp2_x;
	m_y = tmp2_y;
	m_valid = makeSpline(*errorMsg); // we initialize the valid flag to false, so that we enforce a call to makeSpline(). Why not here?

	return m_valid;
}


void LinearSpline::read(const std::string& x_data, const std::string& y_data) {
	std::string errmsg;
	if (!read(x_data, y_data, &errmsg))
		throw IBK::Exception(errmsg, "[LinearSpline::read]");
}


bool LinearSpline::read(std::istream& in, std::string * errorMsg) {
	std::string x_vec, y_vec;
	getline(in, x_vec);
	getline(in, y_vec);
	return read(x_vec, y_vec, errorMsg);
}


void LinearSpline::readBinary(std::istream& in ){
	IBK::read_vector_binary< double >(in, m_x, 10000);
	IBK::read_vector_binary< double >(in, m_y, 10000);
}


void LinearSpline::write(std::ostream& out, unsigned int indent) const {
	std::string istr = std::string(indent, ' ');
	out << istr;
	for (unsigned int i=0; i<m_x.size(); ++i)
		out << std::setw(15) << std::left << m_x[i] << "  ";
	out << std::endl << istr;
	for (unsigned int i=0; i<m_y.size(); ++i)
		out << std::setw(15) << std::left << m_y[i] << "  ";
	out << std::endl;
}


void LinearSpline::writeBinary( std::ostream& out ) const {

	IBK::write_vector_binary< double >(out, m_x);
	IBK::write_vector_binary< double >(out, m_y);

}


void LinearSpline::clear() {
	*this = IBK::LinearSpline();
}


void LinearSpline::swap(LinearSpline& spl) {
	m_x.swap(spl.m_x);
	m_y.swap(spl.m_y);
	m_slope.swap(spl.m_slope);
	std::swap(m_valid, spl.m_valid);
	std::swap(m_extrapolationMethod, spl.m_extrapolationMethod);
	std::swap(m_xMin, spl.m_xMin);
	std::swap(m_xMax, spl.m_xMax);
	std::swap(m_xStep, spl.m_xStep);
	std::swap(m_xOffset, spl.m_xOffset);
}


double LinearSpline::value(double x) const {

	// this does not to be tested in m_valid, since m_extrapolationMethod is public, and of type ExtrapolationMethod
	// thus testing this is not sufficient in make spline, and we need to fix the implementation right here if
	// new types are added to ExtrapolationMethod enum
//	if (m_extrapolationMethod > EM_Constant) {
	IBK_ASSERT_X( (m_extrapolationMethod <= EM_Constant), "Invalid or unknown extrapolation method!" );
//	}
	IBK_ASSERT_X( m_valid, "Linear spline not properly initialized. Call makeSpline() first!" );

	if (m_xStep != 0) {
		if (x>m_xMax) {
			// use extrapolation
		}
		if (x<m_xMin) {
			// use extrapolation
		}
		/// \todo Check this implementation!
		// compute offset
		unsigned int i=(unsigned int)((x-m_xOffset)/m_xStep);
		// ensure that offset is in the valid range
		if (i >= m_slope.size())
			return m_y.back();
		return m_y[i] + m_slope[i]*(x - i*m_xStep);
	}

	if (m_x.size() == 1)
		return m_y[0];
	// x value larger than largest x value?
	if (x > m_x.back()) {
		switch (m_extrapolationMethod) {
			case EM_Constant	: return m_y.back();
			case EM_Linear		: return m_y.back() + m_slope.back()*(x-m_x.back());
		}
	}

	// we use lower bound to find the correct interval
	std::vector<double>::const_iterator it = std::lower_bound(m_x.begin(), m_x.end(), x);
	if (it == m_x.begin()) {
		switch (m_extrapolationMethod) {
			case EM_Constant	: return m_y.front();
			case EM_Linear		: return m_y.front() + m_slope.front()*(x-m_x.front());
		}
	}
	// get the index of the lower limit of the current interval
	unsigned int i = static_cast<unsigned int>(std::distance(m_x.begin(), it) - 1);
#ifdef USE_SLOPE
	return m_y[i] + m_slope[i]*(x - m_x[i]);
#else
	double alpha = (x - m_x[i])/(m_x[i+1]-m_x[i]); // thus must be always between 0 and 1
	IBK_ASSERT(alpha >= 0 && alpha <= 1);
	return m_y[i]*(1-alpha) + m_y[i+1]*alpha;
#endif
}


double LinearSpline::nonInterpolatedValue(double x) const {

	IBK_ASSERT_X( m_valid, "Linear spline not properly initialized. Call makeSpline() first!" );

	if (m_x.size() == 1)
		return m_y[0];
	// x value larger than largest x value?
	if (x > m_x.back())
		return m_y.back();
	// we use lower bound to find the correct interval
	std::vector<double>::const_iterator it = std::lower_bound(m_x.begin(), m_x.end(), x);
	if (it == m_x.begin())
		return m_y.front();
	// get the index of the lower limit of the current interval
	unsigned int i = static_cast<unsigned int>(std::distance(m_x.begin(), it) - 1);
	// special case: x is a saltus of the non-interpolated spline
	// (than equals x the element m_x[i+1])
	if( i < m_x.size() - 1 && x == m_x[i+1] )
		return m_y[i+1];

	return m_y[i];
}


double LinearSpline::slope(double x) const {
	if (!m_valid)
		throw IBK::Exception("Linear spline not properly initialized!","[LinearSpline::value]");
	if (m_x.size() == 1)
		return 0;
	// lower_bound -> i = 0...n   -> subtract 1 to get the slope index
	// max i = n-2 for slope
	size_t i = std::lower_bound(m_x.begin(), m_x.end(), x) - m_x.begin();
	if (i <= 1)					return m_slope.front();
	else if (i < m_x.size()-1)	return m_slope[--i];
	else						return m_slope.back();
}

// *** PRIVATE FUNCTIONS ***

bool LinearSpline::makeSpline(std::string & errMsg) {
	m_valid = false;
	m_slope.clear();
	if (m_x.empty() || m_x.size() != m_y.size()) {
		errMsg = std::string("Invalid vector dimensions.");
		return false;
	}
	if (m_x.size() == 1) {
		// special case, constant spline
		m_valid = true;
		return true;
	}
	// check for strict monotonic increasing values, so if we find any
	// adjacent values that fulfil x[i] >= x[i+1], we have a problem
	std::vector<double>::iterator posIt = std::adjacent_find(m_x.begin(), m_x.end(), std::greater_equal<double>());
	if (posIt != m_x.end()) {
		errMsg = IBK::FormatString("X values are not strictly monotonic increasing (at position/index %1).")
				 .arg((unsigned int)(posIt - m_x.begin())).str();
		return false;
	}
	for (unsigned int i=1; i<m_x.size(); ++i)
		m_slope.push_back( (m_y[i] - m_y[i-1])/(m_x[i]-m_x[i-1]) );
	m_valid = true;
	return true;
}


LinearSpline::SplineGenerationResults LinearSpline::generate(
		const ScalarFunction& f,
		double xMin,
		double xMax,
		double absTol,
		double relTol,
		unsigned int maxPoints,
		bool continueIter)
{
	const char * const FUNC_ID = "[LinearSplineGenerator::generate]";

	// *** PART 1 - Initialization ***

	// adjust xMin and xMax based on existing m_x values
	if (!m_x.empty()) {
		xMin = std::min(xMin, m_x.front());
		xMax = std::max(xMax, m_x.back());
		IBK::IBK_Message( IBK::FormatString("Spline data already existing in generator, using "
											"min/max values: %1/%2").arg(xMin).arg(xMax), IBK::MSG_PROGRESS, FUNC_ID, VL_DETAILED);
	}

	if (xMin >= xMax)
		throw IBK::Exception(IBK::FormatString("Invalid interval (xMin must be < than xMax)!"), FUNC_ID);

	// create temporary copy of vectors
	std::vector<double> xvals = m_x;
	std::vector<double> yvals = m_y;
	xvals.reserve(maxPoints);
	yvals.reserve(maxPoints);

	unsigned int fEvals = 0;

	std::vector<unsigned int> refineInterval;
	refineInterval.reserve(maxPoints);

	// generate start and end point of spline
	if (!continueIter || xvals.size()<2) {
		xvals.clear();
		yvals.clear();
		xvals.push_back( xMin );
		yvals.push_back( f(xMin) );  ++fEvals;
		xvals.push_back( xMax );
		yvals.push_back( f(xMax) );  ++fEvals;
		refineInterval.resize(1, 1); // mark this interval as invalid
	}
	else {
		refineInterval.resize(m_x.size()-1,1); // mark all existing intervals as invalid
	}
	// note: m_x.size() - 1 = refineIntervals.size()

	IBK::IBK_Message(IBK::FormatString("Starting spline generation with %1 points, reltol = %2, abstol = %3.\n")
		.arg((unsigned int)xvals.size()).arg(relTol).arg(absTol),
					 IBK::MSG_PROGRESS, FUNC_ID, 3);


	double xm, ym, ym_approx, dy;

	// infinite while loop
	for (;;) {
		double max_dy = absTol;

		// *** PART 2 - Refinement ***

		// process all intervals marked as "to be refined"
		unsigned int i = 0;
		unsigned int refineCount = 0;
		do {
			// note: i is the interval index, and
			//       i is the index of the left-side point of the interval
			if (refineInterval[i]) {
				// need to check interval
				++refineCount;

				// calculate the exact and approximated y values at the middle
				xm = (xvals[i] + xvals[i+1])/2.0;
				// can we make the interval smaller, still?
				if (xm==xvals[i])
					throw IBK::Exception("Division by zero while adding spline points!", FUNC_ID);
				ym = f(xm);  ++fEvals;
				ym_approx = 0.5*(yvals[i] + yvals[i+1]);
				dy = std::fabs(ym - ym_approx);
				// compute a tolerance that is small compared to ym
				double eps = ym*relTol + absTol;
				// if tolerances are exceeded add the middle point
				if (std::fabs(dy/eps) > 1) {
					max_dy = std::max(std::fabs(dy), max_dy);
					double deltaXleft = std::fabs(xm - xvals[i]);
					double deltaXright = std::fabs(xm - xvals[i+1]);
					double minDelta = std::min(deltaXleft, deltaXright)/std::max(xvals[i],xvals[i+1]);
					// only insert a new interval if xvalues are still spaced enough
					if (minDelta > 1e-11) {
						// we insert a point and a new interval that needs to be refined still
						xvals.insert(xvals.begin()+i+1,1,xm);
						yvals.insert(yvals.begin()+i+1,1,ym);
						// we can only refine the interval further, if the newly computed y values
						// differ enough from the neighboring points
						refineInterval.insert(refineInterval.begin()+i+1,1,1);
						// jump over the newly created interval (we process it in the next loop)
						++i;
					}
					else
						refineInterval[i] = 0;
				}
				else {
					// mark interval as checked
					refineInterval[i] = 0;
				}
			}

			// proceed with next interval
		} while (++i < refineInterval.size());

		IBK::IBK_Message(IBK::FormatString("  Intervals refined = %1, max abs. error = %2, fEvals = %4, current spline size of %3")
			.arg(refineCount).arg(max_dy).arg((unsigned int)xvals.size()).arg(fEvals), IBK::MSG_PROGRESS, FUNC_ID, 3);

		// *** PART 3 - Cleanup ***

		// check all intervals and when a point can be removed without harming interpolation accuracy in the interval, remove the point
		i = 1;
		while (i+1 < refineInterval.size()) {
			// check if yvals[i] can be approximated by linear interpolation between xvals[i-1] and xvals[i+1]
			double dx = xvals[i+1] - xvals[i-1];
			double dy = yvals[i+1] - yvals[i-1];
			double yapprox = yvals[i-1] + (xvals[i] - xvals[i-1])/dx*dy;
			double eps = yapprox*relTol + absTol;
			double delta = yapprox - yvals[i];
			if (std::fabs(delta)/eps < 1) {
				// we can remove the point i
				xvals.erase(xvals.begin()+i);
				yvals.erase(yvals.begin()+i);
				refineInterval.erase(refineInterval.begin()+i);
			}
			else {
				++i; // next interval
			}
		}
		IBK::IBK_Message(IBK::FormatString(" [%1]\n").arg((unsigned int)xvals.size()), IBK::MSG_PROGRESS, FUNC_ID, 3);


		if (refineCount == 0) {
			m_x.swap(xvals);
			m_y.swap(yvals);
			return SG_Ok;
		}

		if (xvals.size() > maxPoints) {
			m_x.swap(xvals);
			m_y.swap(yvals);
			return SG_MaxPointsExceeded;
		}

	} // while (true)
}


}	// namespace IBK

