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

#include <NANDRAD_Schedule.h>

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

	VICUS_READWRITE_OVERRIDE
	VICUS_COMPARE_WITH_ID

	/*! Checks if all referenced schedule is valid. */
	bool isValid(std::string &err,
				 bool checkAnnualScheds = false,
				 const std::map<std::string, IBK::Path> &placeholder = std::map<std::string, IBK::Path>()) const;

	/*! Checks if all referenced schedule is valid. Only for period schedules. */
	bool isValid() const;

	/*! Checks if two schedules encode the same physical behaviour, allowing for small rounding errors. */
	bool isSimilar(const Schedule &other) const;

	/*! Multiply a schedule with another schedule. Returns the result schedule. */
	Schedule multiply(const Schedule &other) const;

	/*! Multiply a schedule with constant value. Returns the result schedule. */
	Schedule multiply(double val) const;

	/*! Multiply a schedule with another schedule. Returns the result schedule. */
	Schedule add(const Schedule &other) const;

	/*! Add a constant value to a schedule. Returns the result schedule. */
	Schedule add(double val) const;

	/*! Create a constant schedule with value val. */
	void createConstSchedule(double val = 0);

	/*! Create a data and a timepoint vector for the hole schedule. Only period based schedules. */
	void createYearDataVector(std::vector<double> &timepoints, std::vector<double> &data) const;

	/*! Creates an annual schedule from a period schedule.
		\param name -> name of quantity
	*/
	Schedule createAnnualScheduleFromPeriodSchedule(std::string &name, const IBK::Unit &unit, unsigned int startDayOfYear = 0);

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;

	/*! Generates NANDRAD schedules from data stored in this object and inserts these into the given schedule group.
		The variable to be inserted into the schedule group is given in parameter 'varName' (formatted including unit).
		Only period schedule are valid.
	*/
	void insertIntoNandradSchedulegroup(const std::string & varName, std::vector<NANDRAD::Schedule> & scheduleGroup) const;

	/*! Generates NANDRAD schedules from data stored in this object and inserts these into the given schedule group.
		The variable to be inserted into the schedule group is given in parameter 'varName' (formatted including unit).
	*/
	void insertIntoNandradSchedulegroup(const std::string & varName, std::vector<NANDRAD::Schedule> & scheduleGroup,
										std::vector<NANDRAD::LinearSplineParameter> &splines,
										const std::map<std::string, IBK::Path> &placeholders = std::map<std::string, IBK::Path>()) const;

	/*! Calculate min and max values of schedule. */
	void calculateMinMax(double &min, double &max) const;

	/*! Converts vector of VICUS day types into a NANDRAD schedule day types
		(also merges weekdays to NANDRAD::Schedule::ST_WEEKDAY, etc.).
	*/
	static std::vector<NANDRAD::Schedule::ScheduledDayType> mergeDayType(const std::vector<int> &dts);

	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int					m_id = INVALID_ID;		// XML:A:required
	//:inherited	IBK::MultiLanguageString		m_displayName;			// XML:A

	/*! Notes. */
	IBK::MultiLanguageString		m_notes;								// XML:E

	/*! Data source. */
	IBK::MultiLanguageString		m_dataSource;							// XML:E

	/*! If true, values are linearly interpolated between given time points.
		Applies to both daily cycle and annual schedules. Defaults to true.
	*/
	bool							m_useLinearInterpolation = false;		// XML:E

	/*! If true, we have an annual schedule (though it might still be empty). */
	bool							m_haveAnnualSchedule = false;			// XML:E

	/*! Annual schedule data. */
	NANDRAD::LinearSplineParameter	m_annualSchedule;						// XML:E

	/*! Data is organized in periods of data.
		Periods in vector must be consecutive in time.
	*/
	std::vector<ScheduleInterval>	m_periods;								// XML:E
};

} // namespace VICUS


#endif // VICUS_ScheduleH
