/*	The NANDRAD data model library.
Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
St. Vogelsang	<stefan.vogelsang -[at]- tu-dresden.de>
All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
*/

#ifndef NANDRAD_LinearSplineParameterH
#define NANDRAD_LinearSplineParameterH

#include <string>

#include <IBK_LinearSpline.h>
#include <IBK_Unit.h>

#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*!	Class LinearSplineParameter stores a linear spline curve, the corresponding parameter name
	and a unit name of both dependend and independend quantity (xUnit, yUnit).

	Note that the read and write functions perform unit conversions on the values, so that
	the linear spline actually holds values always in Base-SI units (according to the
	IBK Unit-Definition).

	So when reading a linear spline that defines temperatures in C, the actual spline will
	then contain the values in K.
*/
class LinearSplineParameter {
public:

	/*! Interpolation method to be used for this linear spline parameter. */
	enum interpolationMethod_t {
		I_CONSTANT,	// Keyword: constant
		I_LINEAR,	// Keyword: linear
		NUM_I
	};

	/*! How to treat the values in multi-year simulations. */
	enum wrapMethod_t {
		C_CONTINUOUS,	// Keyword: continuous		'Continuous data'
		C_CYCLIC,		// Keyword: cyclic			'Annual cycle'
		NUM_C
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE
	NANDRAD_COMP(LinearSplineParameter)

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Parameter name (in context of schedules used as scheduled quantity). */
	std::string				m_name;													// XML:A
	/*! Interpolation method to be used when computing values of this spline. */
	interpolationMethod_t	m_interpolationMethod = I_LINEAR;						// XML:A
	/*! Whether to wrap time around in multi-year simulations (cyclic use) or to assume continuous data. */
	wrapMethod_t			m_wrapMethod = C_CYCLIC;								// XML:A
	/*! Data vectors including linear spline functionality (i.e. interpolation at any given value).
		Values are stored in the respective Base-SI units of the input/output units m_xUnit and
		m_yUnit. For example, if m_yUnit is 'C' (degree C), then the spline holds values in
		'K' (degree Kelvin). The functions readXML() and writeXML() perform the necessary conversions.
	*/
	IBK::LinearSpline		m_values;												// XML:E
	/*! Unit of the x-values. */
	IBK::Unit				m_xUnit;												// XML:A
	/*! Unit of the y-values. */
	IBK::Unit				m_yUnit;												// XML:A
};

} // namespace NANDRAD

#endif // LinearSplineParameterH
