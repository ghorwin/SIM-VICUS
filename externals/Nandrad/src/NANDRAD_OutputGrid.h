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

#ifndef NANDRAD_OutputGridH
#define NANDRAD_OutputGridH

#include <string>
#include <map>

#include "NANDRAD_Interval.h"

class TiXmlElement;

namespace NANDRAD {

/*!	\brief Declaration for class OutputGrid

	A selection of time intervals with a given time step size
	and a reference name.
	Use the reference name to specify the output of simulation results.
*/
class OutputGrid {
public:

	/*! Initializes parametrization defaults. */
	void initDefaults();

	/*! Reads the data from the xml element.
		Throws an IBK::Exception if a syntax error occurs.
	*/
	void readXML(const TiXmlElement * element);
	/*! Appends the element to the parent xml element.
		Throws an IBK::Exception in case of invalid data.
	*/
	void writeXML(TiXmlElement * parent) const;

	/*! Comparison operator by value. */
	bool operator==(const OutputGrid & other) const;

	/*! Returns a parameter of the intervals handled in this OutputGrid, if needed, parameters
		are created/computed on the fly.
	*/
	IBK::Parameter intervalParameter(unsigned int intervalIndex, Interval::para_t p) const;

	/*! Returns a parameter of the last interval handled in this OutputGrid, if needed, parameters
		are created/computed on the fly. Calls \sa intervalParameter().
	*/
	IBK::Parameter lastIntervalParameter(Interval::para_t p) const;


	/*! Checks input parameters and throws an IBK::Exception if
		some input values are wrong. */

	bool checkIntervalDefinition(bool strict) const;

	/*! Computes START parameters in all intervals.
		This is
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
	std::string						m_name;

	/*! The start/duration/end of each interval in [s].
		The time points are defined the same as the simulation time itself.
	*/
	std::vector< Interval >			m_intervals;
};


} // namespace NANDRAD

#endif // ModelH
