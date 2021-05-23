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

#ifndef NANDRAD_IntervalH
#define NANDRAD_IntervalH

#include <string>

#include "NANDRAD_CodeGenMacros.h"

#include <IBK_Parameter.h>

namespace NANDRAD {

/*!	The class Interval defines intervals of simulation time.
	It is used to define output grids and schedules.
*/
class Interval {
	NANDRAD_READWRITE_PRIVATE
public:
	/*! Parameters. */
	enum para_t {
		/*! Start time point. */
		P_Start,		// Keyword: Start		[d] 'Start time point.'
		/*! End time point. */
		P_End,			// Keyword: End			[d] 'End time point.'
		/*! StepSize. */
		P_StepSize,		// Keyword: StepSize	[h] 'StepSize.'
		NUM_P
	};

	NANDRAD_READWRITE_IFNOTEMPTY(Interval)
	NANDRAD_COMP(Interval)

	/*! Convenience function to specify a parameter through start time point and end time point.
		This sets parameters IP_START and IP_END to given values using the unit u for both parameters.
		\param start The start time point, expected to be already in the unit u.
		\param endtime The end time point, expected to be already in the unit u. If endtime should not
			be specified, given std::numeric_limits<double>::max() as value.
		\param u Time unit of start and endtime.
	*/
	void setStartEnd( double start, double endtime, IBK::Unit u );

	/*! Checks input parameters.
		Definition of Start and End intervals are optional. If provided, the values must be >= 0
		and End time point must be always beyond start time point.
		Throws an IBK::Exception if conditions are not met.
	*/
	void checkParameters() const;

	/*! Returns true, if t lies inside the internal.
		This check requires a valid START parameter to be set.
		\param t Simulation time, in [s], simtime definition.
		\warning This function expects checkParameters() to be called beforehand.
	*/
	bool isInInterval(double t) const;

	/*! Returns the end time point.
		It returns the END parameter if given. Otherwise infinitiy.
		\warning This function expects checkParameters() to be called beforehand.
	*/
	double endTime() const;

	/*! The parameters defining the interval. */
	IBK::Parameter						m_para[NUM_P];		// XML:E
};

} // namespace NANDRAD

#endif // NANDRAD_IntervalH
