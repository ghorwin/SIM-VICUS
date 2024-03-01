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

#ifndef VICUS_ScheduleIntervalH
#define VICUS_ScheduleIntervalH

#include <IBK_MultiLanguageString.h>

#include "VICUS_CodeGenMacros.h"

#include <vector>

#include "VICUS_DailyCycle.h"

namespace VICUS {

/*! Describes a period within a schedule where course of scheduled quantities is defined through
	sets of daily cycles.
*/
class ScheduleInterval {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE

	/*! Checks if all referenced materials exist and if their parameters are valid. */
	bool isValid() const;

	/*! Check if all day type are used in a Schedule interval. Option with or without holyday. */
	bool usedAllDayTypes(bool withHolyday = true) const;

	/*! Returns a set of all unused day types. */
	std::set<int> freeDayTypes();

	/*! Multiply to schedule intervals and create a new one. */
	ScheduleInterval multiply(const ScheduleInterval &other, unsigned int startDay) const;

	/*! Multiply a value to schedule intervals and create a new one. */
	ScheduleInterval multiply(double val)const;

	/*! Add schedule intervals and create a new one. */
	ScheduleInterval add(const ScheduleInterval &other, unsigned int startDay) const;

	/*! Add a value to a schedule interval and returns the result. */
	ScheduleInterval add(double val) const;

	/*! Create a schedule interval with a constant value for all days. */
	void createConstScheduleInterval(double val = 0);

	/*! Create a set of time points and corresponding values for an entire week.
		The week starts at monday and ends at sunday. The spacing between time points is arbitrary.
		The generated vectors are guaranteed to have at least one point each.
	*/
	void createWeekDataVector(std::vector<double> &timepoints, std::vector<double> &data) const;

	/*! Calculate min and max values of schedule interval. */
	void calculateMinMax(double &min, double &max) const;

	/*! Comparsion operator. */
	bool operator!=(const ScheduleInterval &other) const;

	/*! Comparsion operator. */
	bool operator==(const ScheduleInterval &other) const {return !(*this != other); }

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Display name of period. */
	IBK::MultiLanguageString	m_displayName;									// XML:A


	/*! Day index (0) where schedule period starts (max. 364). First period always starts with 0.
		Each period lasts until begin of next interval, or until end of year (if it is the last period).
	*/
	unsigned int				m_intervalStartDay=0;							// XML:A

	/*! Vector with daily cycles. */
	std::vector<DailyCycle>		m_dailyCycles;									// XML:E
};

} // namespace VICUS


#endif // VICUS_ScheduleIntervalH
