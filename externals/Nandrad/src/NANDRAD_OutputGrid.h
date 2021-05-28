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

#ifndef NANDRAD_OutputGridH
#define NANDRAD_OutputGridH

#include <string>

#include "NANDRAD_Interval.h"
#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*!	An OutputGrid defines time intervals with a given output step size per interval. */
class OutputGrid {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE
	NANDRAD_COMP(OutputGrid)
	NANDRAD_COMPARE_WITH_NAME

	/*! Checks input parameters and throws an IBK::Exception if some input values are wrong.
		\note This function should be called after readXML() during solver initialization, and before
			  setupIntervals() is called.
	*/
	void checkIntervalDefinition() const;

	/*! Returns a parameter of the intervals handled in this OutputGrid, if needed, parameters
		are created/computed on the fly.
	*/
	IBK::Parameter intervalParameter(unsigned int intervalIndex, Interval::para_t p) const;

	/*! Returns a parameter of the last interval handled in this OutputGrid, if needed, parameters
		are created/computed on the fly. Calls \sa intervalParameter().
	*/
	IBK::Parameter lastIntervalParameter(Interval::para_t p) const;

	/*! Computes START parameters in all intervals.
		Call this function after interval parameters have been checked with checkIntervalDefinition().
	*/
	void setupIntervals();

	/*! Returns true, if the time point t matches an
		output grid point defined by this grid.
		\param t Time point as absolute time offset to Midnight, January 1st of the start year.
		\warning This function expects that checkParameters() was previously called.
	*/
	bool isActive(double t) const;

	/*! Computes and returns next output time point past tOutCurrent that is scheduled
		with this output grid.
	*/
	double computeNextOutputTime(double tOutCurrent) const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Descriptive name for this output grid. */
	std::string						m_name;									// XML:A

	/*! The start/duration/end of each interval in [s].
		The time points are defined the same as the simulation time itself.
	*/
	std::vector<Interval>			m_intervals;							// XML:E
};


} // namespace NANDRAD

#endif // NANDRAD_OutputGridH
