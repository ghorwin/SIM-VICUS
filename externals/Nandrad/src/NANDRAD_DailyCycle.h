/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
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

/*!	Defines the daily course of one or more physical quantities/setpoints.

	The m_interpolation parameter defines how the tabulated data is interpreted.

	If m_interpolation == IT_CONSTANT, then the following rules apply:
	- the time points in m_timePoints are interpreted as *start* of the next interval
	- the first time point must be always 0, the last one must be < 24 h,
	- the corresponding value is taken as constant during this interval

	For example, a time point vector "0 6 18" defines three intervals: 0-6, 6-18, 18-24 and
	the data table must contain exactly 3 values.

	If m_interpolation == IT_LINEAR, then the following rules apply:
	- the time points in m_timePoints are points in time where associated values are given
	- the first time point must be always 0, the last one must be < 24 h,
	  because in cyclic usage, the time point at 24 h will be the same as for 0 h (and likewise
	  the scheduled values)
	- between time points the values are linearly interpolated
*/
class DailyCycle {
public:

	/*! Interpolation method for daily cycle data.
		Note that for constant values some ramping may be used to smoothen out the steps.
		If not set, IT_LINEAR is used.
	*/
	enum interpolation_t {
		/*! Constant values in defined intervals. */
		IT_Constant,	// Keyword: Constant	'Constant values in defined intervals.'
		/*! Linear interpolation between values. */
		IT_Linear,		// Keyword: Linear		'Linear interpolation between values.'
		NUM_IT
	};

	/*! This structure holds the data for a single parameter stored in this DailyCycle.
		It is used in vector m_valueData which is populated in prepareCalculation().
	*/
	struct valueData_t {
		/*! Default C'tor. */
		valueData_t() {}
		/*! Initializing C'tor. */
		valueData_t(const std::string & name, const IBK::Unit & unit, const std::vector<double>	* valueVec) :
			m_name(name), m_unit(unit), m_valueVec(valueVec) {}

		/*! Comparison operator. */
		bool operator==(const std::string & name) const { return m_name == name; }

		/*! Parameter name. */
		std::string					m_name;
		/*! Value input/output unit. */
		IBK::Unit					m_unit;
		/*! Pointer to vector containing the actual data. */
		const std::vector<double>	*m_valueVec = nullptr;
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE
	NANDRAD_COMP(DailyCycle)

	/*! Checks for valid parametrization and generates m_valueNames, m_valueUnits and m_valueData. */
	void prepareCalculation();

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Value interpolation method. */
	interpolation_t						m_interpolation = NUM_IT;			// XML:A

	/*! Time points in [h], must be strictly monotonically increasing, first must be 0, last must be less than 24. */
	std::vector<double>					m_timePoints;						// XML:E

	/*! Actual values, key of m_values.m_values is physical quantity/setpoint/... scheduled quantity,
		value is vector with values, same number of values as in m_timePoints.
	*/
	DataTable							m_values;							// XML:E

	// *** solver runtime variables only (not written to file) ***

	/*! Extracted data of all variables. */
	std::vector<valueData_t>			m_valueData;
};

} // namespace NANDRAD

#endif // NANDRAD_DailyCycleH
