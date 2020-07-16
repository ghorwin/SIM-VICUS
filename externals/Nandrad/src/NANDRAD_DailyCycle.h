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

#ifndef NANDRAD_DailyCycleH
#define NANDRAD_DailyCycleH

#include <string>
#include <vector>

#include <IBK_Parameter.h>
#include "NANDRAD_LinearSplineParameter.h"
#include "NANDRAD_DataTable.h"
#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*!	\brief Declaration for class DailyCycle

	A daily cycle provides the values of a scheduled quantity for the period of one day.
	It uses a vector of intervals to specify time and values of the scheduled quantity.
	Usually more than one quantities are defined within one interval.

	The interval definition defines a time interval per default. The last interval of one
	daily cycle does not need a time definition - it lasts until the end of day. The m_para
	section of the interval provides information about the interval time, the m_genericParaConst
	section provides the scheduled parameters with a constant value for each interval.

	Alternatively scheduled quantities may directly be defined as a linear spline parameter. Use
	the time in hours (for one complete day) as x-values and the hourly values of the quantity
	as y-values for the spline.
*/
class DailyCycle {
public:

	/*! Interpolation method for daily cycle data.
		Note that for constant values some ramping may be used to smoothen out the steps.
	*/
	enum interpolation_t {
		IT_CONSTANT,	// Keyword: CONSTANT	'Constant hourly values, required 24 values in vectors.'
		IT_LINEAR,		// Keyword: LINEAR		'Linear interpolation between values, requires timepoints from 0h to 24h.'
		NUM_IT
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE
	NANDRAD_COMP(DailyCycle)

	/*! Converts for interals or hourly values to a linear splin for a given quantity:
		Returns an empty linear spline if quantity is undefined.*/
	void createLinearSpline(const std::string &quantityName, IBK::LinearSpline &spline) const;

	/*! Checks input intervals and throws an IBK::Exception if
		some interval definition does not match definition rules. */
	void checkIntervalDefinition() const;

	/*! return the interval end in seconds for given interval index, if needed, parameters are conmputed on the fly. */
	double	intervalEndInSeconds( unsigned int intervalIndex ) const;

	// *** PUBLIC MEMBER VARIABLES ***

	interpolation_t						m_interpolation = NUM_IT;			// XML:A

	/*! Time points, first must be 0, last must be end of day.
		For hourly constant values, time unit must be 'h', and exactly 24 values must be in spline (0...23).
	*/
	std::vector<double>					m_timePoints;						// XML:E
	IBK::Unit							m_timeUnit;							// XML:E

	/*! Actual values, key of m_values.m_values is physical quantity/setpoint/... scheduled quantity, value is vector with values, same
		number of values as in m_timePoints.
	*/
	DataTable							m_values;							// XML:E
};

} // namespace NANDRAD

#endif // DailyCycleH
