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

Schedule Schedule::multiply(const Schedule &other) const {
	FUNCID("Schedule::multiply");

	Schedule sched;

	if(!isValid()){
		//Schedule '%1' with (id=%2) is not valid.
		return sched;
	}

	if(!other.isValid()){
		//Schedule '%1' with (id=%2) is not valid.
		return sched;
	}
	if(m_useLinearInterpolation != other.m_useLinearInterpolation)
		throw IBK::Exception(IBK::FormatString("Schedule with id %1 and %2 have different interpolation method."
												" Multiply is not possible").arg(m_id).arg(other.m_id), FUNC_ID);
	//make a copy of the other schedule to the new schedule
	sched = other;
	//compare periods
	unsigned int j=0;
	unsigned int i=0;
	while(i<sched.m_periods.size()){
		const ScheduleInterval &period = m_periods[j];
		ScheduleInterval period2 = sched.m_periods[i];
		//for same start day make a multiply
		if(period.m_intervalStartDay == period2.m_intervalStartDay){
			if(j+1 >= m_periods.size())
				//fertig
				break;
			++j;
		}
		//check for insert
		else if(period.m_intervalStartDay > period2.m_intervalStartDay){
			bool haveNext = i+1<sched.m_periods.size();
			//insert a period from this schedule in the sched
			if((haveNext && period.m_intervalStartDay < sched.m_periods[i+1].m_intervalStartDay) || !haveNext){
				period2.m_intervalStartDay = period.m_intervalStartDay;
				sched.m_periods.insert(sched.m_periods.begin()+i+1, period2);
			}
			++i;
		}
	}
	//multi
	i=0;
	j=0;
	//now sched holds all start days
	//iterate sched periods and check for same or lower startdays in m_periods
	//if true multiply
	//otherwise increment
	while(i<sched.m_periods.size()){
		const ScheduleInterval &period = m_periods[j];
		ScheduleInterval period2 = sched.m_periods[i];

		if(period2.m_intervalStartDay == period.m_intervalStartDay ||
				(period2.m_intervalStartDay > period.m_intervalStartDay &&
				 j+1<m_periods.size() &&
				 period2.m_intervalStartDay < m_periods[j+1].m_intervalStartDay) ||
				j+1 >= m_periods.size()){
			sched.m_periods[i] = sched.m_periods[i].multiply(m_periods[j], sched.m_periods[i].m_intervalStartDay);
			++i;
		}
		else if(j+1<m_periods.size() &&
				period2.m_intervalStartDay >= m_periods[j+1].m_intervalStartDay){
			++j;
		}
	}
	///TODO Dirk->Andreas der neu erstellte Zeitplan hat jetzt die gleiche ID wie der OTHER
	/// damit kracht es jetzt
	/// Wie unterbinde ich das?
	/// Wie finde ich eine Unique Id?
	return sched;
}

Schedule Schedule::multiply(double val) const {
	FUNCID("Schedule::multiply");

	Schedule sched;

	if(!isValid()){
		//Schedule '%1' with (id=%2) is not valid.
		return sched;
	}

	if(val < 0)
		IBK::Exception(IBK::FormatString("Multiply negative values to a schedule is not allowed."), FUNC_ID);

	//make a copy of the other schedule to the new schedule
	sched = *this;
	//multi
	//now sched holds all start days
	//iterate sched periods and check for same or lower startdays in m_periods
	//if true multiply
	//otherwise increment

	for(unsigned int i=0; i<sched.m_periods.size(); ++i)
		sched.m_periods[i] = sched.m_periods[i].multiply(val);

	///TODO Dirk->Andreas der neu erstellte Zeitplan hat jetzt die gleiche ID wie der OTHER
	/// damit kracht es jetzt
	/// Wie unterbinde ich das?
	/// Wie finde ich eine Unique Id?
	return sched;
}

AbstractDBElement::ComparisonResult Schedule::equal(const AbstractDBElement *other) const {
	const Schedule * otherSched = dynamic_cast<const Schedule*>(other);
	if (otherSched  == nullptr)
		return Different;

	//first check critical data

	if(m_useLinearInterpolation != otherSched ->m_useLinearInterpolation ||
			m_annualSchedule != otherSched ->m_annualSchedule ||
			m_periods != otherSched ->m_periods )
		return Different;

	//check meta data

	if(m_displayName != otherSched ->m_displayName ||
			m_notes != otherSched ->m_notes ||
			m_dataSource != otherSched ->m_dataSource )
		return OnlyMetaDataDiffers;

	return Equal;
}


} // namespace VICUS
