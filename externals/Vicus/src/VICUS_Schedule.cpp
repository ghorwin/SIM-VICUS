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

#include "VICUS_Schedule.h"
#include <algorithm>

namespace VICUS {

// Returns the possible merged day types. If no merge is possible returns dts.
std::vector<NANDRAD::Schedule::ScheduledDayType> Schedule::mergeDayType(const std::vector<int> &dts) {
	FUNCID(Schedule::mergeDayType);
	// we must have always have a non-empty vector
	if (dts.empty())
		throw IBK::Exception("Expected non-empty vector with day types.", FUNC_ID);

	std::set<NANDRAD::Schedule::ScheduledDayType> dayTypes;

	// insert all daytypes in vector into set
	unsigned int weekDayCount = 0;
	unsigned int weekEndDayCount = 0;
	for (int v : dts) {
		switch ((NANDRAD::Schedule::ScheduledDayType)v) {
			case NANDRAD::Schedule::ST_MONDAY:
			case NANDRAD::Schedule::ST_TUESDAY:
			case NANDRAD::Schedule::ST_WEDNESDAY:
			case NANDRAD::Schedule::ST_THURSDAY:
			case NANDRAD::Schedule::ST_FRIDAY:
				++weekDayCount;
			break;
			case NANDRAD::Schedule::ST_SATURDAY:
			case NANDRAD::Schedule::ST_SUNDAY:
				++weekEndDayCount;
			break;
			case NANDRAD::Schedule::ST_HOLIDAY:
			case NANDRAD::Schedule::ST_ALLDAYS:
			case NANDRAD::Schedule::ST_WEEKDAY:
			case NANDRAD::Schedule::ST_WEEKEND:
			case NANDRAD::Schedule::NUM_ST:
			break;
		}
		dayTypes.insert((NANDRAD::Schedule::ScheduledDayType)v);
	}

	// replace weekend days if both present with ST_WEEKEND
	if (weekEndDayCount + weekDayCount == 7)
		return std::vector<NANDRAD::Schedule::ScheduledDayType> {NANDRAD::Schedule::ST_ALLDAYS};
	if (weekEndDayCount == 2) {
		dayTypes.erase(NANDRAD::Schedule::ST_SATURDAY);
		dayTypes.erase(NANDRAD::Schedule::ST_SUNDAY);
		dayTypes.insert(NANDRAD::Schedule::ST_WEEKEND);
	}
	if (weekDayCount == 5) {
		dayTypes.erase(NANDRAD::Schedule::ST_MONDAY);
		dayTypes.erase(NANDRAD::Schedule::ST_TUESDAY);
		dayTypes.erase(NANDRAD::Schedule::ST_WEDNESDAY);
		dayTypes.erase(NANDRAD::Schedule::ST_THURSDAY);
		dayTypes.erase(NANDRAD::Schedule::ST_FRIDAY);
		dayTypes.insert(NANDRAD::Schedule::ST_WEEKDAY);
	}

	std::vector<NANDRAD::Schedule::ScheduledDayType> schedDts(dayTypes.begin(), dayTypes.end());

	return schedDts;
}


bool Schedule::isValid(bool checkAnnualScheds, const std::map<std::string, IBK::Path> &placeholder) const {

	if (m_haveAnnualSchedule) {
		if (!m_annualSchedule.m_tsvFile.isValid()){

			// check if we have data
			if (m_annualSchedule.m_values.empty())
				return false;
			if (!m_annualSchedule.m_values.valid())
				return false;
			// check that we have x and y unit set correctly
			if (m_annualSchedule.m_xUnit.base_id() != IBK_UNIT_ID_SECONDS)
				return false;
			// yUnit is not important -> Model defines unit later
		}
		else if(checkAnnualScheds){
			// Normally, we should check if the file exists, if number of columns match etc., but that is
			// pretty time-consuming and isValid() is being called very often. So we check this
			// during schedule export.

			// Also, we have no way of resolving the absolute path from a relative path, since we don't
			// have access to the project's file path.


			// load the data
			NANDRAD::LinearSplineParameter spline;
			IBK::Path filepath = m_annualSchedule.m_tsvFile.withReplacedPlaceholders(placeholder);
			spline = m_annualSchedule;
			spline.readTsv(placeholder,1);

			// check if we have data
			if (spline.m_values.empty())
				return false;
			if (!spline.m_values.valid())
				return false;
			// check that we have x and y unit set correctly
			if (spline.m_xUnit.base_id() != IBK_UNIT_ID_SECONDS)
				return false;
			// yUnit is not important -> Model defines unit later

			// TODO Dirk->Andreas wie kann ich die const Funktion umgehen und doch was reinschreiben?
		}





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
	FUNCID(Schedule::multiply(Schedule));

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
	return sched;
}

Schedule Schedule::multiply(double val) const {
	FUNCID(Schedule::multiply(double));

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

	return sched;
}

Schedule Schedule::add(double val) const{
//	FUNCID(Schedule::add(double));

	Schedule sched;

	if(!isValid()){
		//Schedule '%1' with (id=%2) is not valid.
		return sched;
	}

	//make a copy of the other schedule to the new schedule
	sched = *this;

	for(unsigned int i=0; i<sched.m_periods.size(); ++i)
		sched.m_periods[i] = sched.m_periods[i].add(val);

	return sched;
}

void Schedule::createConstSchedule(double val) {
	ScheduleInterval si;
	si.createConstScheduleInterval(val);
	m_periods.push_back(si);
}

void Schedule::createYearDataVector(std::vector<double> &timepoints, std::vector<double> &data) const {
	if (m_periods.empty())
		return;

	for (unsigned int i=0; i<m_periods.size(); ++i) {
		const ScheduleInterval &si = m_periods[i];
		unsigned int dayCount =0;
		unsigned int startDay = si.m_intervalStartDay%7;		// 0 -> Monday, 1 -> Tuesday, ... , 6 -> Sunday
		//find next period to get the size of days
		if (i+1 == m_periods.size()){
			//now we have last period
			dayCount = 365 - si.m_intervalStartDay;
		}
		else{
			dayCount = m_periods[i+1].m_intervalStartDay - si.m_intervalStartDay;
			// FIXME Dirk: can this become negative? This is nowhere checked!
		}
		unsigned int endDayFirstWeek = std::min<unsigned int>(7, startDay + dayCount);
		//get week time and data points
		std::vector<double> tp, d, tpSum, dSum;
		si.createWeekDataVector(tp,d);
		if (tp.empty())
			continue;

		unsigned int addedDays = 0;
		unsigned int weekCount = si.m_intervalStartDay/7  ;

		//add first days to fill up current week
		if (startDay > 0 ){
			for (unsigned int n=0; n<tp.size(); ++n){
				if (tp[n] >= endDayFirstWeek * 24 )
					break;
				addedDays = (unsigned int)(tp[n] / 24 - startDay);
				if(tp[n] >= startDay * 24){
					timepoints.push_back(tp[n] + weekCount*7*24);
					data.push_back(d[n]);
				}
			}
		}
		// no days left take next period
		if (addedDays > dayCount)
			continue;
		weekCount = (dayCount- addedDays)/7  ;
		unsigned int weekOfYear = (si.m_intervalStartDay + addedDays )/7;

		for (unsigned int n=0; n<weekCount; ++n){
			++weekOfYear;
			std::vector<double> newTps = tp;
			for (unsigned int j=0; j<newTps.size(); ++j)
				newTps[j] += weekOfYear * 24 * 7;
			timepoints.insert(timepoints.end(), newTps.begin(), newTps.end());
			data.insert(data.end(), d.begin(), d.end());
			addedDays += 7;
		}
		if (weekOfYear > 0) {
			++weekOfYear;
			++addedDays;
		}
		// check if we have days left
		if (addedDays > dayCount)
			continue;

		// add left days to timepoints/data vector
		unsigned int addedDays2=0;
		for (unsigned int n=0; n<tp.size(); ++n){
			addedDays2 = (unsigned int)(tp[n] / 24);
			if (addedDays2 >= dayCount - addedDays)
				break;
			timepoints.push_back(tp[n] + weekOfYear*7*24);
			data.push_back(d[n]);
		}
	}
}

Schedule Schedule::createAnnualScheduleFromPeriodSchedule(std::string &name, const IBK::Unit & unit, unsigned int startDayOfYear) {
	FUNCID(Schedule::createAnnualScheduleFromPeriodSchedule);
	if(m_periods.empty())
		return Schedule();

	// save start day of year for shifting later
	unsigned int startDayOfPeriod = startDayOfYear;
	std::vector<double> values;
	for(unsigned int i=0; i<365; ++i){
		for(unsigned int iPeriod = 0; iPeriod < m_periods.size(); ++i){

			ScheduleInterval &sv = m_periods[iPeriod];

			// find next period start day
			unsigned int lastPeriodDay = 364;
			if(iPeriod+1 > m_periods.size())
				lastPeriodDay = m_periods[iPeriod+1].m_intervalStartDay - 1;
			// get the hole days of these period
			unsigned int days = lastPeriodDay - sv.m_intervalStartDay + 1;

			unsigned int dayCounter = 0;
			//create a week value vector with 7*24 values or short if period is less than 7 days
			std::vector<double> weekVals;
			for(unsigned int iDay = 0; iDay<7; ++i){
				if(iDay > days)
					break;
				// search for the days
				for(unsigned int iDailyCylce = 0; iDailyCylce<sv.m_dailyCycles.size(); ++iDailyCylce){
					DailyCycle &dc = sv.m_dailyCycles[iDailyCylce];
					int dayType = NANDRAD::Schedule::NUM_ST;
					switch (startDayOfPeriod) {
						case 0:		dayType = NANDRAD::Schedule::ST_MONDAY;		break;
						case 1:		dayType = NANDRAD::Schedule::ST_TUESDAY;	break;
						case 2:		dayType = NANDRAD::Schedule::ST_WEDNESDAY;	break;
						case 3:		dayType = NANDRAD::Schedule::ST_THURSDAY;	break;
						case 4:		dayType = NANDRAD::Schedule::ST_FRIDAY;		break;
						case 5:		dayType = NANDRAD::Schedule::ST_SATURDAY;	break;
						case 6:		dayType = NANDRAD::Schedule::ST_SUNDAY;		break;
					}

					// if day type is not in this daily cycle check next one
					if(!dc.containsDaytype(dayType))
						continue;

					// create 24 hours for time point vector
					if(dc.m_values.empty())
						throw IBK::Exception(IBK::FormatString("Schedule has no values."), FUNC_ID);
					std::vector<double>	vals(24, dc.m_values[0]) ;

					for(unsigned int j=1; j<dc.m_timePoints.size(); ++j){
						for(unsigned int j2=0; j2<24; ++j2){
							if(j2 < dc.m_timePoints[j])
								continue;
							vals[j2] = dc.m_values[j];
						}
					}

					weekVals.insert(weekVals.end(), vals.begin(), vals.end());
					++dayCounter;
					break;
				}

				// add the values to annual sched
				values.insert(values.end(), weekVals.begin(), weekVals.end());
				while (dayCounter<days) {
					unsigned int diff = days - dayCounter;
					if(diff >= 7) {
						values.insert(values.end(), weekVals.begin(), weekVals.end());
						dayCounter +=7;
					}
					else
						values.insert(values.end(), weekVals.begin(), weekVals.begin()+ 24 * diff);
				}
			}
			// set up new start day
			startDayOfPeriod += days;
			startDayOfPeriod = startDayOfPeriod % 7;
		}
	}

	std::vector<double> timepoints(8760);
	for(unsigned int i=0; i<timepoints.size(); ++i)
		timepoints[i] = i;

	Schedule sched;
	// TODO Dirk die Interpolationsmethode wird derzeit nur bei konstant richtig betrachtet sonst müsste man es oben noch umstellen
	sched.m_useLinearInterpolation = false;
	sched.m_annualSchedule = NANDRAD::LinearSplineParameter(name, NANDRAD::LinearSplineParameter::I_CONSTANT,timepoints, values,
															IBK::Unit("h"),IBK::Unit("C"));
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

void Schedule::insertIntoNandradSchedulegroup(const std::string &varName, std::vector<NANDRAD::Schedule> &scheduleGroup) const{
	if (scheduleGroup.empty()){
		// create for each period a new NANDRAD schedule
		for (unsigned int i=0; i<m_periods.size(); ++i){
			const ScheduleInterval &period = m_periods[i];
			// find day types for NANDRAD schedule
			for (unsigned int j=0; j<period.m_dailyCycles.size(); ++j){
				const DailyCycle &dc = period.m_dailyCycles[j];

				// merge all possible day types
				std::vector<NANDRAD::Schedule::ScheduledDayType> dts = mergeDayType(dc.m_dayTypes);

				// create for each day type in merge vector a new NANDRAD schedule
				for (NANDRAD::Schedule::ScheduledDayType dt : dts){
					NANDRAD::Schedule s;
					//set up start day
					if (period.m_intervalStartDay > 0)
						s.m_startDayOfTheYear = period.m_intervalStartDay;
					// set up end day
					// search in next period for a start day
					if (i+1 < m_periods.size())
						s.m_endDayOfTheYear = m_periods[i+1].m_intervalStartDay - 1;
					s.m_type = dt;
					NANDRAD::DailyCycle dcNANDRAD;
					dcNANDRAD.m_interpolation = m_useLinearInterpolation ? NANDRAD::DailyCycle::IT_Linear : NANDRAD::DailyCycle::IT_Constant;
					dcNANDRAD.m_timePoints = dc.m_timePoints;
					dcNANDRAD.m_values.m_values[varName] = dc.m_values;
					s.m_dailyCycles.push_back(dcNANDRAD);
					scheduleGroup.push_back(s);
				}
			}
		}
	}
	else {
		for (unsigned int i=0; i<m_periods.size(); ++i){
			const VICUS::ScheduleInterval &period = m_periods[i];

			for (unsigned int j=0; j<period.m_dailyCycles.size(); ++j){
				const VICUS::DailyCycle &dc = period.m_dailyCycles[j];

				// merge all possible day types
				std::vector<NANDRAD::Schedule::ScheduledDayType> dts = mergeDayType(dc.m_dayTypes);

				// loop over all day types of vicus schedule
				for(NANDRAD::Schedule::ScheduledDayType dt : dts){

					bool valuesAdded = false;
					// check if a period with equal start+end date exists
					for(NANDRAD::Schedule &schedNandrad : scheduleGroup){

						//now check day types of vicus schedule with nandrad schedule
						//for a nandrad schedule following properties must be equal
						// * day type
						// * start day
						// * end day
						// if this is all equal we can add the timepoint and values to an existing daily cycle with same interpolation method
						// otherwise we add a new daily cycle to the daily cylce vector

						if(dt == schedNandrad.m_type &&
								schedNandrad.m_startDayOfTheYear == period.m_intervalStartDay &&
								((i+1<m_periods.size() && schedNandrad.m_endDayOfTheYear == m_periods[i+1].m_intervalStartDay-1) ||
								 (schedNandrad.m_endDayOfTheYear == 364 && m_periods.size() == 1))){
							//now check if we have daily cylces with equal properties to:
							// * interpolation method
							// * time points
							for(NANDRAD::DailyCycle &dcNandrad : schedNandrad.m_dailyCycles){
								if( ( (dcNandrad.m_interpolation == NANDRAD::DailyCycle::IT_Constant && !m_useLinearInterpolation) ||
									  (dcNandrad.m_interpolation == NANDRAD::DailyCycle::IT_Linear && m_useLinearInterpolation) ) &&
										dcNandrad.m_timePoints == dc.m_timePoints){
									// now we can add the data
									dcNandrad.m_values.m_values[varName] = dc.m_values;
									valuesAdded = true;
									break;
								}
							}
							//nothing found to add data
							//create a new daily cycle
							if(!valuesAdded){
								NANDRAD::DailyCycle newDcNandrad;
								newDcNandrad.m_interpolation = m_useLinearInterpolation ? NANDRAD::DailyCycle::IT_Linear : NANDRAD::DailyCycle::IT_Constant;
								newDcNandrad.m_timePoints = dc.m_timePoints;
								newDcNandrad.m_values.m_values[varName] = dc.m_values;
								schedNandrad.m_dailyCycles.push_back(newDcNandrad);
								valuesAdded = true;
							}
						}
						if(valuesAdded)
							break;
					}
					//no schedule found
					//so add a new one
					if(!valuesAdded){
						NANDRAD::Schedule newNandradSched;
						newNandradSched.m_startDayOfTheYear = period.m_intervalStartDay;
						if(i+1<m_periods.size())
							newNandradSched.m_endDayOfTheYear = m_periods[i+1].m_intervalStartDay - 1;

						//create daily cyle
						NANDRAD::DailyCycle newDcNandrad;
						newDcNandrad.m_interpolation = m_useLinearInterpolation ? NANDRAD::DailyCycle::IT_Linear : NANDRAD::DailyCycle::IT_Constant;
						newDcNandrad.m_timePoints = dc.m_timePoints;
						newDcNandrad.m_values.m_values[varName] = dc.m_values;
						//add daily cycle to schedule
						newNandradSched.m_dailyCycles.push_back(newDcNandrad);

						for(NANDRAD::Schedule::ScheduledDayType dtNandrad : dts){
							newNandradSched.m_type = dtNandrad;
							//add schedule to schedule group
							if(!newNandradSched.m_dailyCycles.empty())
								scheduleGroup.push_back(newNandradSched);
						}
					}
				}
			}
		}
	}
}

void Schedule::insertIntoNandradSchedulegroup(const std::string & varName, std::vector<NANDRAD::Schedule> & scheduleGroup,
											  std::vector<NANDRAD::LinearSplineParameter> &splines) const {

	if(m_haveAnnualSchedule){
		std::string::size_type pos, pos2;
		pos = varName.find_last_of("[");
		pos2 = varName.find_last_of("]");
		Q_ASSERT(pos != std::string::npos && pos2 != std::string::npos && pos < pos2);
		std::string name = varName.substr(0, pos-1);
		std::string unitName = varName.substr(pos+1, pos2-pos-1);
		NANDRAD::LinearSplineParameter spline(name,
											  m_useLinearInterpolation ? NANDRAD::LinearSplineParameter::I_LINEAR :
																		 NANDRAD::LinearSplineParameter::I_CONSTANT,
											  m_annualSchedule.m_values.x(),
											  m_annualSchedule.m_values.y(),
											  m_annualSchedule.m_xUnit, IBK::Unit(unitName));
		splines.push_back(spline);
	}
	else
		insertIntoNandradSchedulegroup(varName, scheduleGroup);
}


} // namespace VICUS
