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

#include <limits>

#include "NANDRAD_OutputGrid.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_KeywordList.h"

#include <IBK_assert.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <IBK_messages.h>

namespace NANDRAD {


bool OutputGrid::operator!=(const OutputGrid & other) const {
	if (m_name != other.m_name) return true;
	if (m_intervals != other.m_intervals) return true;
	return false;
}


IBK::Parameter OutputGrid::intervalParameter(unsigned int intervalIndex, Interval::para_t p) const {
	IBK_ASSERT(intervalIndex < m_intervals.size());

	std::string kwName = KeywordList::Keyword("Interval::para_t", p);
	IBK::Parameter par = m_intervals[intervalIndex].m_para[p];

	switch (p) {
		case Interval::P_StepSize :
			// if the stepsize for the requested interval is not yet defined, we default it to 1 h
			if (par.name.empty()) {
				par.set(kwName, "1 h" );
			}
		break;

		case Interval::P_Start :
			if (par.name.empty()) {
				// start with special handling of first interval
				if (intervalIndex == 0)
					par.set(kwName, "0 h" );
				else {
					IBK_ASSERT( !m_intervals[intervalIndex-1].m_para[Interval::P_End].name.empty() );
					par = m_intervals[intervalIndex-1].m_para[Interval::P_End];
				}
			}
		break;

		case Interval::P_End :
			if (par.name.empty()) {
				// start with special handling of last interval
				if (intervalIndex+1 == m_intervals.size())
					par.set(kwName, std::numeric_limits<double>::max(), "h" );
				else {
					IBK_ASSERT( !m_intervals[intervalIndex+1].m_para[Interval::P_Start].name.empty() );
					par = m_intervals[intervalIndex+1].m_para[Interval::P_Start];
				}
			}
		break;
		default:
			IBK_ASSERT_X(false, "Invalid argument p to OutputGrid::intervalParameter");
	}
	return par;
}


IBK::Parameter OutputGrid::lastIntervalParameter(Interval::para_t p) const {
	return intervalParameter( ((unsigned int) m_intervals.size() - 1), p );
}


void OutputGrid::setupIntervals() {
	if (m_intervals.empty())
		return;

	// special case for first interval: set start parameter to 0 if not specified
	if (m_intervals[0].m_para[NANDRAD::Interval::P_Start].name.empty()) {
		m_intervals[0].m_para[NANDRAD::Interval::P_Start].set(
			NANDRAD::KeywordList::Keyword("Interval::para_t", NANDRAD::Interval::P_Start), 0, "s");
	}

	// check that intervals are defined consecutively and that all parameters make sense
	double startTime = 0;
	for (unsigned int i = 0; i < m_intervals.size(); ++i) {
		Interval &interval = m_intervals[i];

		// compute interval start
		if (interval.m_para[Interval::P_Start].name.empty()) {
			// if in first interval, interval starts at time 0
			startTime = 0;
			if (i == 0)
				startTime = 0;
			else {
				// in other intervals
				IBK_ASSERT(!m_intervals[i - 1].m_para[Interval::P_End].name.empty());
				startTime = m_intervals[i - 1].m_para[Interval::P_End].value;
			}
			std::string startTimeName = KeywordList::Keyword("Interval::para_t", Interval::P_Start);
			// set start time
			interval.m_para[Interval::P_Start].set(startTimeName, startTime, "s");
		}
		else {
			startTime = interval.m_para[Interval::P_Start].value;
		}

		// now compute end time
		// we require either End parameter for this, or Start parameter for next interval
		if (interval.m_para[Interval::P_End].name.empty()) {

			double endTime = std::numeric_limits<double>::max();
			// no end time - this is only acceptable in the last interval
			// for all other intervals, we require a start value in the next interval
			if (i + 1 != m_intervals.size()) {
				IBK_ASSERT(!m_intervals[i + 1].m_para[Interval::P_Start].name.empty());
				endTime = m_intervals[i + 1].m_para[Interval::P_Start].value;
			}
			std::string endTimeName = KeywordList::Keyword("Interval::para_t", Interval::P_End);
			// set end time
			interval.m_para[Interval::P_End].set(endTimeName, endTime, "s");
		}
	}
}


void OutputGrid::checkIntervalDefinition() const {
	FUNCID(OutputGrid::checkParameters);

	if (m_intervals.empty())
		throw IBK::Exception( IBK::FormatString("Output Grid '%1' does not have any intervals.").arg(m_name), FUNC_ID);

	double startTime = 0; // assume first interval starts at time 0 (offset to Midnight January 1st of the start year)

	// check that intervals are defined consecutively and that all parameters make sense
	for (unsigned int i = 0; i < m_intervals.size(); ++i) {
		const Interval &interval = m_intervals[i];

		// check all parameters in the interval for valid inputs
		interval.checkParameters();

		// check if stepSize parameter is defined
		if (interval.m_para[Interval::P_StepSize].name.empty()) {
			throw IBK::Exception( IBK::FormatString("Interval#%2 in output grid '%1' does not have a StepSize parameter.")
								  .arg(m_name).arg(i+1), FUNC_ID);
		}
		else if (interval.m_para[Interval::P_StepSize].value <= 0) {
			throw IBK::Exception( IBK::FormatString("StepSize parameter of interval #%1 in output grid '%2' is <= 0 (which is invalid).")
				.arg(i+1).arg(m_name), FUNC_ID);
		}

		// compute interval start
		if (interval.m_para[Interval::P_Start].name.empty()) {
			// if start is missing in first interval, interval starts at time 0
			if (i == 0)
				startTime = 0;
			else {
				// check if end time is given in last interval
				if (m_intervals[i-1].m_para[Interval::P_End].name.empty()) {
					throw IBK::Exception( IBK::FormatString("'End' parameter in interval #%1, or Start parameter in "
															"interval #%2 is required in output grid '%3'.")
											.arg(i).arg(i+1).arg(m_name), FUNC_ID);
				}
				else {
					// use end time point of last interval as start time for next interval
					startTime = m_intervals[i-1].m_para[Interval::P_End].value;
				}
			}
		}
		else {
			// use start time of current interval
			startTime = interval.m_para[Interval::P_Start].value;
		}

		// compute interval end
		double endTime = std::numeric_limits<double>::max();

		// we require either End parameter for this, or Start parameter for next interval
		if (interval.m_para[Interval::P_End].name.empty()) {
			// in the last interval, the interval lasts forever, in all others we need to look
			// ahead at the next Interval's Start parameter
			if (i+1 != m_intervals.size()) {
				if (m_intervals[i+1].m_para[Interval::P_Start].name.empty()) {
					throw IBK::Exception( IBK::FormatString("'End' parameter in interval #%1, or Start parameter in "
															"interval #%2 is required in output grid '%3'.")
												.arg(i+1).arg(i+2).arg(m_name), FUNC_ID);
				}
				else
					endTime = m_intervals[i+1].m_para[Interval::P_Start].value;
			}
			// else {
				// Note: endTime is already initialized with std::numeric_limits<double>::max();
			// }
		}
		else {
			endTime = interval.m_para[Interval::P_End].value;
		}

		// now check that endTime  > startTime
		if (endTime <= startTime) {
			throw IBK::Exception( IBK::FormatString("Interval #%1 in output grid '%2' has a Start that lies past the interval End.")
								  .arg(i+1).arg(m_name), FUNC_ID);
		}

	}
}


bool OutputGrid::isActive(double t) const {
	for (unsigned int i=0; i<m_intervals.size(); ++i) {
		if (m_intervals[i].isInInterval(t)) {
			double stepsize = m_intervals[i].m_para[Interval::P_StepSize].value;
			double t_offset = t - m_intervals[i].m_para[Interval::P_Start].value;
			// get the step number
			unsigned int step_number = (unsigned int)std::floor(t_offset/stepsize);
			// re-compute the step-time point
			double step_time = step_number * stepsize + m_intervals[i].m_para[Interval::P_Start].value;
			// the grid is active, if the computed step_size is approximately the same as t
			return IBK::f_fuzzyEQ(step_time, t);
		}
	}
	return false; // none found
}


double OutputGrid::computeNextOutputTime(double tOutCurrent) const {
	// process all intervals and find the interval that encloses the current time
	// point tOutCurrent
	for (unsigned int i=0; i<m_intervals.size(); ++i) {
		if (m_intervals[i].isInInterval(tOutCurrent)) {
			// tOutCurrent lies within this interval

			// compute offset to start
			double t_offset = tOutCurrent - m_intervals[i].m_para[Interval::P_Start].value;
			// t_offset must be >= 0
			// get the step number
			double stepsize = m_intervals[i].m_para[Interval::P_StepSize].value;
			unsigned int step_number = (unsigned int)std::floor(t_offset/stepsize) + 1;
			// compute the corresponding output time for this step
			double tOutNext = step_number * stepsize + m_intervals[i].m_para[Interval::P_Start].value;
			// if we are directly at this time point, we need to advance to the next
			while (IBK::f_fuzzyLTEQ(tOutNext, tOutCurrent))
				tOutNext += stepsize;
			// if the computed output time point is in or at the end of this interval, we return
			double tEnd = m_intervals[i].endTime();
			if (tOutNext < tEnd || IBK::f_fuzzyEQ(tOutNext, tEnd)) {
				return tOutNext;
			}
			// continue to next interval
		}
		else {
			// if current time point is before this interval, return this time point
			if (tOutCurrent < m_intervals[i].m_para[Interval::P_Start].value)
				return m_intervals[i].m_para[Interval::P_Start].value;

			// the current time point is already past the intervals end, continue to next interval
		}
	}
	return std::numeric_limits<double>::max();
}


} // namespace NANDRAD

