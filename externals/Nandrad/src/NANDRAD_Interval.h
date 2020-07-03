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

#ifndef NANDRAD_IntervalH
#define NANDRAD_IntervalH

#include <string>
#include <iosfwd>
#include <vector>

#include <IBK_Parameter.h>
#include <IBK_math.h>

class TiXmlElement;

namespace NANDRAD {

/*!	The class Interval defines intervals of simulation time.
	It is used to defined output grids or other intervals.
*/
class Interval {
public:
	/*! Parameters. */
	// ***KEYWORDLIST-START***
	enum para_t {
		IP_START,		// Keyword: Start		[d] 'Start time point.'
		IP_END,			// Keyword: End			[d] 'End time point.'
		IP_STEPSIZE,	// Keyword: StepSize	[h] 'StepSize.'
		NUM_IP
	};
	// ***KEYWORDLIST-END***

	/*! Reads the data from the xml element.
		Throws an IBK::Exception if a syntax error occurs.
	*/
	void readXML(const TiXmlElement * element);

	/*! Appends the element to the parent xml element.
		Throws an IBK::Exception in case of invalid data.
	*/
	void writeXML(TiXmlElement * parent, bool detailedOutput) const;

	/*! Compares this instance with another by value and returns true if they differ. */
	bool operator!=(const Interval & other) const;

	/*! Compares this instance with another by value and returns true if they are the same. */
	bool operator==(const Interval & other) const { return ! operator!=(other); }

	/*! Convenience function to specify a parameter through start time point and end time point.
		This sets parameters IP_START and IP_END to given values using the unit u for both parameters.
		\param start The start time point, expected to be already in the unit u.
		\param end The end time point, expected to be already in the unit u. If endtime should not
			be specified, given std::numeric_limits<double>::max() as value.
	*/
	void setStartEnd( double start, double endtime, IBK::Unit u );

	/*! Checks input parameters.
		Definition of Start and End intervals are optional. If provided, the values must be >= 0
		and End time point must be always beyond start time point.
		\param strict If true, an IBK::Exception is thrown whenever input values are wrong. If strict
				is false, an error message is written to IBK::IBK_Message() for each error encountered
				and the function returns false.
		\return Returns true if all parameters are value. If strict is set to false, an error in the
				parameters will cause the function to return with false.
	*/
	bool checkParameters(bool strict) const;

	/*! Returns true, if t lies inside the internal.
		This check requires a valid START parameter to be set.
		\warning This function expects checkParameters() to be called beforehand.
	*/
	bool isInInterval(double t) const;

	/*! Returns the end time point.
		It returns the END parameter if given. Otherwise infinitiy.
		\warning This function expects checkParameters() to be called beforehand.
	*/
	double endTime() const;

	/// \todo we shouldn't use ibk parameter here, doesn't make sense, and we lose
	/// 50 percent of the data type size

	/*! The parameters defining the interval. */
	IBK::Parameter						m_para[NUM_IP];
};

} // namespace NANDRAD

#endif // NANDRAD_IntervalH
