/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

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

#ifndef VICUS_DailyCycleH
#define VICUS_DailyCycleH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"

#include <vector>
#include <string>

namespace VICUS {

/*! Defines time points and values for a single day, and also defines, for which day of the week (day type)
	this time series shall apply to.
*/
class DailyCycle {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMP(DailyCycle)

	/*! Checks if all referenced materials exist and if their parameters are valid. */
	bool isValid() const;

	/*! Check if a day type contains the m_dayTypes vector. */
	bool containsDaytype(int dt);

	/*! Multiplies current daily cycle with other daily cycle and generates a new daily cycle that contains
		the union of all time points.
		Only those day types will be in the generated DC that are present in both the current and other daily cycle.
	*/
	DailyCycle multiply(const DailyCycle &other) const;

	/*! Scale all values of the daily cycle with a constant factor. */
	DailyCycle multiply(double factor) const;

	/*! Adds current daily cycle to other daily cycle and generates a new daily cycle that contains
		the union of all time points.
		Only those day types will be in the generated DC that are present in both the current and other daily cycle.
	*/
	DailyCycle add(const DailyCycle &other) const;

	/*! Adds a value to the daily cycle. */
	DailyCycle add(double val) const;

	/*! Calculate min and max values of daily cycle. */
	void calculateMinMax(double &min, double &max) const;

	/*! Multiply operator. */
	DailyCycle operator *(const DailyCycle &other) const {return multiply(other);}

	/*! Multiply operator. */
	DailyCycle operator *(double val) const {return multiply(val);}

	/*! Add operator. */
	DailyCycle operator +(const DailyCycle &other) const {return add(other);}

	/*! Add operator. */
	DailyCycle operator +(double val) const {return add(val);}

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Holds one or more day types from NANDRAD::Schedule::type_t enum.
		Only the days (Mo, ... , Fr, .. So) and ST_HOLIDAY.
	*/
	std::vector<int>		m_dayTypes;																// XML:E

	/*! Vector with time points. First value must always be 0 h. */
	std::vector<double>		m_timePoints;															// XML:E

	/*! Vector with corresponding values (same size as m_timePoints vector).
		When data is interpreted as linearly interpolated, the values are taken as given directly
		at the matching time points. Otherwise, the value is taken constant during the interval starting
		at the corresponding time point.
	*/
	std::vector<double>		m_values;																// XML:E

	/*! Holds error string in order to give users a tooltip in db dialog. */
	mutable std::string		m_errorMsg;
};

} // namespace VICUS


#endif // VICUS_DailyCycleH
