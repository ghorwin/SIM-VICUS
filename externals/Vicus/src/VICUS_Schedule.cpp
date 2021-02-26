#include "VICUS_Schedule.h"

namespace VICUS {

bool Schedule::isValid() const {

	if (!m_annualSchedule.empty()) {
		// if we have annual schedule data, we must not have periods at the same time
		if (!m_periods.empty())
			return false;

		// TODO : Annual schedule check

		return true;
	}


	// *** daily cycle based schedule check ***

	// check that the period start days are increasing

	// we must have at least one period
	if (m_periods.empty())
		return false;

	// first periods must have valid parameters itself
	if (!m_periods[0].isValid())
		return false;

	// first period must start at day 0
	if (m_periods.front().m_intervalStartDay != 0)
		return false;

	unsigned int lastIntervalStart = 0;
	// check that all intervals follow each other
	for (unsigned int i=1; i<m_periods.size(); ++i) {
		// period must have valid parameters itself
		if (!m_periods[i].isValid())
			return false;
		if (m_periods[i].m_intervalStartDay <= lastIntervalStart)
			return false;
		lastIntervalStart = m_periods[i].m_intervalStartDay;
	}

	return true;
}

bool Schedule::multiply(Schedule &other) {
	///TODO Katja

	//compare periods

	//add all periods to a new schedule

	//multiply all values in loops
	return true;
}



} // namespace VICUS
