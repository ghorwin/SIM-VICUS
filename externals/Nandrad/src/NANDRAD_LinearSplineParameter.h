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

/*!	\brief Declaration for class LinearSplineParameter

	A linear spline parameter stores a linear spline curve, the corresponding parameter name
	and a unit name of both dependend and independend quantity (xUnit, yUnit).

	Note that the read and write functions perform unit conversions on the values, so that
	the linear spline actually holds values always in Base-SI units (according to the
	IBK Unit-Definition).
*/
class LinearSplineParameter {
public:

	/*! Interpolation method to be used for this linear spline parameter. */
	enum interpolationMethod_t {
		I_Constant,	// Keyword: constant
		I_Linear,	// Keyword: linear
		NUM_I
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Reads the data from the xml element.
		Throws an IBK::Exception if a syntax error occurs.
	*/
	void readXML(const TiXmlElement * element);
	/*! Appends the element to the parent xml element.
		Throws an IBK::Exception in case of invalid data.
	*/
	void writeXML(TiXmlElement * parent) const;

	/*! Compares this instance with another by value and returns true if they differ. */
	bool operator!=(const LinearSplineParameter & other) const;
	/*! Compares this instance with another by value and returns true if they are the same. */
	bool operator==(const LinearSplineParameter & other) const { return ! operator!=(other); }

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Parameter name.*/
	std::string				m_name;
	/*! Interpolation method to be used when computing values of this spline. */
	interpolationMethod_t	m_interpolationMethod = I_Linear;
	/*! Data vectors including linear spline functionality (i.e. interpolation at any given value).
		Values are stored in the respective Base-SI units of the input/output units m_xUnit and
		m_yUnit. For example, if m_yUnit is 'C' (degree C), then the spline holds values in
		'K' (degree Kelvin). The functions readXML() and writeXML() perform the necessary conversions.
	*/
	IBK::LinearSpline		m_values;
	/*! Unit of the x-values. */
	IBK::Unit				m_xUnit;
	/*! Unit of the y-values. */
	IBK::Unit				m_yUnit;
};

} // namespace NANDRAD

#endif // LinearSplineParameterH
