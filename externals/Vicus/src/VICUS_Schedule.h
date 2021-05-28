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

#ifndef VICUS_ScheduleH
#define VICUS_ScheduleH

#include <IBK_LinearSpline.h>

#include <IBK_Flag.h>
#include <IBK_Parameter.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_ScheduleInterval.h"
#include "VICUS_AbstractDBElement.h"

namespace VICUS {

/*! Describes the course of a *single* scheduled quantity (basically a value over time data set).
 *  This schedule does not have a unit.
*/
class Schedule : public AbstractDBElement {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	/*! Checks if all referenced schedule is valid. */
	bool isValid() const;

	/*! Multiply a schedule with another schedule. */
	Schedule multiply(const Schedule &other) const;

	Schedule multiply(double val) const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of Schedule. */
	unsigned int					m_id = INVALID_ID;					// XML:A:required

	/*! Display name of Schedule. */
	IBK::MultiLanguageString		m_displayName;						// XML:A

	/*! Notes. */
	IBK::MultiLanguageString		m_notes;							// XML:E

	/*! Data source. */
	IBK::MultiLanguageString		m_dataSource;						// XML:E

	/*! If true, values are linearly interpolated between given time points.
		Applies to both daily cycle and annual schedules. Defaults to true.
	*/
	bool							m_useLinearInterpolation = true;	// XML:E

	/*! Annual schedules are simply stored as linear spline. */
	IBK::LinearSpline				m_annualSchedule;					// XML:E

	/*! Data is organized in periods of data.
		Periods in vector must be consecutive in time.
	*/
	std::vector<ScheduleInterval>	m_periods;							// XML:E



};

} // namespace VICUS


#endif // VICUS_ScheduleH
