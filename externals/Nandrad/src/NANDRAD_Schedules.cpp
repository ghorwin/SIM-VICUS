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

#include <algorithm>

#include "NANDRAD_Schedules.h"
#include "NANDRAD_KeywordList.h"
#include "NANDRAD_Utilities.h"
#include "NANDRAD_Constants.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <IBK_messages.h>
#include <IBK_UnitVector.h>

#include <tinyxml.h>

namespace NANDRAD {


void Schedules::initDefaults() {
	m_flags[F_EnableCyclicSchedules].set(
				NANDRAD::KeywordList::Keyword("Schedules::flag_t", F_EnableCyclicSchedules), true);
}

void Schedules::checkParameters(const std::map<std::string, IBK::Path> &placeholders) {
	FUNCID(Schedules::checkParameters);

	// process all annual splines
	for (std::map<std::string, std::vector<NANDRAD::LinearSplineParameter> >::iterator it = m_annualSchedules.begin();
		 it != m_annualSchedules.end(); ++it)
	{
		for (NANDRAD::LinearSplineParameter & spl : it->second) {
			try {
				spl.m_tsvFile = spl.m_tsvFile.withReplacedPlaceholders(placeholders);
				// all checks will be skipped, if a file name was given: only reads the file and converts to base units.
				// Since we skip the unit check, we need to pass dummy units here, that are, however, baseSI units ...
				spl.checkAndInitialize("", IBK::Unit("s"), IBK::Unit("s"), IBK::Unit("s"), 0, false, 0, false, nullptr, true);
			} catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Error initializing annual spline schedule data for schedule '%1'.")
									 .arg(spl.m_name), FUNC_ID);
			}

			// still we check that x value unit is indeed convertible to time
			if (spl.m_xUnit.base_id() != IBK_UNIT_ID_SECONDS) {
				if (spl.m_tsvFile.isValid())
					throw IBK::Exception(IBK::FormatString("Invalid time unit '%2' in tsv-file '%1'.")
										 .arg(spl.m_tsvFile).arg(spl.m_xUnit.name()), FUNC_ID);
				else
					throw IBK::Exception(IBK::FormatString("Invalid time unit '%2' in AnnualSchedule '%1'.")
										 .arg(spl.m_name).arg(spl.m_xUnit.name()), FUNC_ID);
			}
		}
	}
	// now all linear splines (annual data) are properly initialized - unit check and variable name check is done
	// when schedule object is initialized
}


void Schedules::readXML(const TiXmlElement * element) {
	FUNCID(Schedules::readXML);

	// reading elements
	const TiXmlElement * c = element->FirstChildElement();
	while (c) {
		const std::string & cName = c->ValueStr();
		if (cName == "Holidays") {
			IBK::Time::TimeFormatInfo fmt = IBK::Time::formatInfo("dd.MM.");
			const TiXmlElement * c2 = c->FirstChildElement();
			while (c2) {
				const std::string & c2Name = c2->ValueStr();
				if (c2Name != "FirstDayOfYear")
					throw IBK::Exception(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), FUNC_ID);

				IBK::Time t = IBK::Time::fromString(c2->GetText(), fmt, false);
				if (!t.isValid())
					throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(c2->Row())
										 .arg("Invalid date format '"+std::string(c2->GetText())+"', expected date in format 'dd.MM.'."), FUNC_ID);
				unsigned int dayOfYear = static_cast<unsigned int>(t.secondsOfYear()/(3600*24));
				if (m_holidays.find(dayOfYear) != m_holidays.end())
					throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(c2->Row())
										 .arg("Duplicate holiday date '"+std::string(c2->GetText())+"'"), FUNC_ID);
				m_holidays.insert(dayOfYear);

				c2 = c2->NextSiblingElement();
			}

		}
		else if (cName == "WeekEndDays") {
			std::string days = c->GetText();
			// split days at ,
			std::vector<std::string> dayvec;
			IBK::explode(days, dayvec, ",", IBK::EF_TrimTokens);
			// convert all days into enums
			for (const std::string & d : dayvec) {
				try {
					day_t dt = (day_t)KeywordList::Enumeration("Schedules::day_t", d);
					m_weekEndDays.insert(dt);
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception(ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
										 .arg("Invalid day name in 'WeekEndDays' tag."), FUNC_ID);
				}
			}
		}
		else if (cName == "FirstDayOfYear") {
			std::string firstDay = c->GetText();
			// convert first day of year
			try {
				m_firstDayOfYear = (day_t)KeywordList::Enumeration("Schedules::day_t", firstDay);
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
									 .arg("Invalid day name in 'FirstDayOfYear' tag."), FUNC_ID);
			}
		}
		else if (cName == "IBK:Flag") {
			IBK::Flag f;
			readFlagElement(c, f);
			bool success = false;
			try {
				flag_t ftype = (flag_t)KeywordList::Enumeration("Schedules::flag_t", f.name());
				m_flags[ftype] = f; success=true;
			}
			catch (...) { /* intentional fail */  }
			if (!success)
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_NAME).arg(f.name()).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
		}
		else if (cName == "ScheduleGroups") {
			const TiXmlElement * c2 = c->FirstChildElement();
			while (c2) {
				const std::string & c2Name = c2->ValueStr();
				if (c2Name != "ScheduleGroup")
					throw IBK::Exception(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), FUNC_ID);

				const TiXmlAttribute * attrib = TiXmlAttribute::attributeByName(c2, "objectList");
				if (attrib == nullptr)
					throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(c2->Row()).arg(
						"Missing required 'objectList' attribute."), FUNC_ID);

				std::string objectListName = attrib->ValueStr();
				// ensure that we do not have duplicate definitions
				if (m_scheduleGroups.find(objectListName) != m_scheduleGroups.end())
					throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(c2->Row()).arg(
						"ObjectList '"+objectListName+"' used for multiple ScheduleGroup elements."), FUNC_ID);

				std::vector<Schedule> schedules;
				// now read all the schedule subtags

				const TiXmlElement * c3 = c2->FirstChildElement();
				while (c3) {
					const std::string & c3Name = c3->ValueStr();
					if (c3Name != "Schedule")
						throw IBK::Exception(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c3Name).arg(c3->Row()), FUNC_ID);

					Schedule s;
					try {
						s.readXML(c3);
					} catch (IBK::Exception & ex) {
						throw IBK::Exception(ex, "Error reading 'ScheduleGroup' element.", FUNC_ID);
					}
					schedules.push_back(s);

					c3 = c3->NextSiblingElement();
				}

				m_scheduleGroups[objectListName] = schedules;

				c2 = c2->NextSiblingElement();
			}
		}
		else if (cName == "AnnualSchedules") {
			const TiXmlElement * c2 = c->FirstChildElement();
			while (c2) {
				const std::string & c2Name = c2->ValueStr();
				if (c2Name != "ScheduleGroup")
					throw IBK::Exception(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c2Name).arg(c2->Row()), FUNC_ID);

				const TiXmlAttribute * attrib = TiXmlAttribute::attributeByName(c2, "objectList");
				if (attrib == nullptr)
					throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(c2->Row()).arg(
						"Missing required 'objectList' attribute."), FUNC_ID);

				std::string objectListName = attrib->ValueStr();
				// ensure that we do not have duplicate definitions
				if (m_annualSchedules.find(objectListName) != m_annualSchedules.end())
					throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(c2->Row()).arg(
						"ObjectList '"+objectListName+"' used for multiple AnnualSchedules elements."), FUNC_ID);

				std::vector<NANDRAD::LinearSplineParameter> schedules;
				// now read all the schedule subtags

				const TiXmlElement * c3 = c2->FirstChildElement();
				while (c3) {
					const std::string & c3Name = c3->ValueStr();
					if (c3Name != "AnnualSchedule")
						throw IBK::Exception(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(c3Name).arg(c3->Row()), FUNC_ID);

					NANDRAD::LinearSplineParameter spl;
					try {
						spl.readXML(c3);	// also creates the spline and thus checks for valid spline
											// Note: in case of tsv file those checks will be done later in Schedules::checkParameters()
					}
					catch (IBK::Exception & ex) {
						throw IBK::Exception(ex, IBK::FormatString(XML_READ_ERROR).arg(c2->Row())
											 .arg("Invalid data in 'AnnualSchedule' tag."), FUNC_ID);
					}

					schedules.push_back(spl);

					c3 = c3->NextSiblingElement();
				}

				m_annualSchedules[objectListName] = schedules;

				c2 = c2->NextSiblingElement();
			}
		}
		else {
			IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
		}
		c = c->NextSiblingElement();
	}
}


TiXmlElement * Schedules::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("Schedules");
	parent->LinkEndChild(e);

	if (!m_holidays.empty()) {
		TiXmlElement * c = new TiXmlElement("Holidays");
		e->LinkEndChild(c);
		// encode days
		std::string holidays;
		for (unsigned int t : m_holidays)
			holidays += std::string(",") + IBK::val2string(t);
		TiXmlText * text = new TiXmlText( holidays.substr(1) ); // Mind: remove leading , from string
		c->LinkEndChild(text);
	}

	if (!m_weekEndDays.empty()) {
		TiXmlElement * c = new TiXmlElement("WeekEndDays");
		e->LinkEndChild(c);
		// encode days
		std::string days;
		for (day_t t : m_weekEndDays)
			days += std::string(",") + KeywordList::Keyword("Schedules::day_t", t);
		TiXmlText * text = new TiXmlText( days.substr(1) ); // Mind: remove leading , from string
		c->LinkEndChild(text);
	}

	if (m_firstDayOfYear != Schedules::SD_MONDAY) {
		TiXmlElement * c = new TiXmlElement("FirstDayOfYear");
		e->LinkEndChild(c);
		// encode days
		std::string firstDay = KeywordList::Keyword("Schedules::day_t", m_firstDayOfYear);
		TiXmlText * text = new TiXmlText( firstDay ); // Mind: remove leading , from string
		c->LinkEndChild(text);
	}

	for (int i=0; i<NUM_F; ++i) {
		if (!m_flags[i].name().empty())
			TiXmlElement::appendSingleAttributeElement(e, "IBK:Flag", "name", m_flags[i].name(), m_flags[i].isEnabled() ? "true" : "false");
	}

	// now write schedules
	if (!m_scheduleGroups.empty()) {
		TiXmlElement * c = new TiXmlElement("ScheduleGroups");
		e->LinkEndChild(c);
		for (const std::pair<const std::string, std::vector<Schedule> > & svec : m_scheduleGroups) {
			// create tags like
			// <ScheduleGroup objectList="objectListName">
			//   <Schedule>...</Schedule>
			// </ScheduleGroup>
			TiXmlElement * g = new TiXmlElement("ScheduleGroup");
			c->LinkEndChild(g);
			g->SetAttribute("objectList", svec.first);
			for (const Schedule & s : svec.second)
				s.writeXML(g);
		}
	}

	// now write schedules
	if (!m_annualSchedules.empty()) {
		TiXmlElement * c = new TiXmlElement("AnnualSchedules");
		e->LinkEndChild(c);
		for (const std::pair<const std::string, std::vector<NANDRAD::LinearSplineParameter> > & svec : m_annualSchedules) {
			// create tags like
			// <ScheduleGroup objectList="objectListName">
			//   ...
			// </ScheduleGroup>
			TiXmlElement * g = new TiXmlElement("ScheduleGroup");
			c->LinkEndChild(g);
			g->SetAttribute("objectList", svec.first);
			for (const NANDRAD::LinearSplineParameter & s : svec.second) {
				TiXmlElement * splTag = s.writeXML(g);
				// Mind: we expect an 'AnnualSchedule' tag instead of 'LinearSplineParameter'
				splTag->ToElement()->SetValue("AnnualSchedule");
			}
		}
	}

	return nullptr;
}


bool Schedules::operator!=(const Schedules & other) const {
	if (m_holidays != other.m_holidays) return true;
	if (m_weekEndDays != other.m_weekEndDays) return true;
	if (m_scheduleGroups != other.m_scheduleGroups) return true;
	if (m_annualSchedules != other.m_annualSchedules) return true;
	return false;
}


void Schedules::generateLinearSpline(const std::string & objectListName, const std::string & parameterName,
									 IBK::LinearSpline & spline, DailyCycle::interpolation_t & interpolationType) const
{
	FUNCID(Schedules::generateLinearSpline);

	/*
		- loop over all days (d=0,1,...,364)
		- determine day type:
		  dayOfWeek = (startDayOffset + d) % 7 (modulo 7)
		- look up daily cycle:
		  - find schedule where d in range:
			- process daytypes in order Holidays, CurrentDayType, Weekdays/Weekends, AllDays
			  - if parameter is found in any of these days, take daily course and add to spline for this day,
			  - if parameter not found, skip and search through next schedule
	*/

	interpolationType = DailyCycle::NUM_IT; // initialize with invalid interpolation mode first, will be set and compared below

	// TIME_SHIFT defines the minimum gap introduced between two values in the spline,
	// to avoid extreme gradients and thus slowdown of simulation.
	// Note: if someone started a new interval just shortly (< TIME_SHIFT) before
	//       end of the day, this will generate a non-monotonic time vector which
	//       will blow up in our face once we generate the linear spline below.
	//       Thankfully, we catch that and can inform the user about this parameter
	//       problem.
	const double TIME_SHIFT = 120; // in [s]


	std::vector<double> tp;
	IBK::UnitVector vals;
	// loop over all days
	for (unsigned int d=0; d<365; ++d) {
		// compute date type
		day_t dayOfWeek = (day_t)((d + m_firstDayOfYear) % 7);

		// also determine if this day is a holiday
		bool isHoliday = (m_holidays.find(d) != m_holidays.end());

		// get vector of schedules for given object list
		std::map<std::string, std::vector<Schedule> >::const_iterator it = m_scheduleGroups.find(objectListName);
		IBK_ASSERT(it != m_scheduleGroups.end());
		const std::vector<Schedule> & schedules = it->second;

		std::vector<const Schedule *> scheduleCandidates[NANDRAD::Schedule::NUM_ST];

		// now search through schedules and build a list of schedules sorted according to priority (i.e. daytype)
		for (const Schedule & sched : schedules) {
			// check for correct interval
			if (!sched.containsDay(d))
				continue; // outside scheduled date range - skip

			// look at schedule day type and see if our day fits
			bool keep = false;
			switch (sched.m_type) {
				// AllDays are always kept
				case NANDRAD::Schedule::ST_ALLDAYS :
					keep = true;
				break;

				// holidays are only kept if this is a holiday schedule
				case NANDRAD::Schedule::ST_HOLIDAY :
					if (isHoliday)
						keep = true;
				break;

				case NANDRAD::Schedule::ST_WEEKDAY :
					// weekday schedules are only kept, if daytype is a weekday
					if (dayOfWeek >= NANDRAD::Schedules::SD_MONDAY &&
						dayOfWeek <= NANDRAD::Schedules::SD_FRIDAY)
					{
						keep = true;
					}
				break;

				case NANDRAD::Schedule::ST_WEEKEND :
					// weekend schedules are only kept, if daytype is a weekend
					if (dayOfWeek >= NANDRAD::Schedules::SD_SATURDAY &&
						dayOfWeek <= NANDRAD::Schedules::SD_SUNDAY)
					{
						keep = true;
					}
				break;

				default :
					// otherwise we only keep the schedule if this is a "our" day
					if (sched.m_type == (int)dayOfWeek+NANDRAD::Schedule::ST_MONDAY) // mind the shift in enum numbers
						keep = true;
			}

			if (!keep)
				continue; // schedule was filtered out, skip it

			// check if schedule provides the requested parameter
			// Note: this is a slow operation, so we do it last
			if (sched.m_parameters.find(parameterName) == sched.m_parameters.end())
				continue; // parameter not provided

			// remember based on date type
			scheduleCandidates[sched.m_type].push_back(&sched);
		}

		// now we have a set of schedules to take our parameter from
		// each of the lists should have at max one parameter - otherwise we have an ambiguity

		// check all lists types for duplicate schedules
		for (unsigned int schedDayType=0; schedDayType<NANDRAD::Schedule::NUM_ST; ++schedDayType) {
			IBK::Time dString(2003, (d + m_firstDayOfYear)*IBK::SECONDS_PER_DAY);
			if (scheduleCandidates[schedDayType].size() > 1) {
				// we allow at max. 2 schedules, but only if exactly one of the
				// two is a whole year schedule
				// so, we basically count the number of whole year schedules
				// and bail out if we have more than one, otherwise we subtract
				// the number of all year schedules from the number of schedules
				// and check, that 0 or 1 remain
				unsigned int allYearCount = 0;
				for (const NANDRAD::Schedule * schedPtr : scheduleCandidates[schedDayType]) {
					if (schedPtr->isWholeYearSchedule())
						++allYearCount;
				}
				if (allYearCount > 1)
					throw IBK::Exception(IBK::FormatString("Ambiguous schedule parameter definition for "
														   "parameter '%1'. Multiple whole year schedules match this day.")
										 .arg(parameterName), FUNC_ID);
				if (scheduleCandidates[schedDayType].size() - allYearCount > 1)
					throw IBK::Exception(IBK::FormatString("Ambiguous schedule parameter definition for parameter '%1' at day '%2'. %3 "
										 "schedules of day type '%4' match this day.")
										 .arg(parameterName).arg(dString.toDayMonthFormat())
										 .arg(scheduleCandidates[schedDayType].size())
										 .arg(NANDRAD::KeywordList::Keyword("Schedule::ScheduledDayType", (int)schedDayType)), FUNC_ID);
			}
		}

		// search backwards so that we find the highest priority schedule
		bool dataAdded = false;
		for (int i=NANDRAD::Schedule::NUM_ST-1; i>=0; --i) {
			if (scheduleCandidates[i].empty())  continue; // skip day types without schedule
			const NANDRAD::Schedule * sched = scheduleCandidates[i].front();
			// priority handling for the case that we have both an whole year schedule and a limited schedule
			if (scheduleCandidates[i].size() > 1) {
				if (sched->isWholeYearSchedule())
					sched = scheduleCandidates[i].back();
			}
			// find daily cycle that provides the parameter
			std::map<std::string, NANDRAD::DailyCycle *>::const_iterator paraIT = sched->m_parameters.find(parameterName);
			IBK_ASSERT(paraIT != sched->m_parameters.end());

			const NANDRAD::DailyCycle * dc = paraIT->second;
			// retrieve daily cycle parameter data
			const std::vector<NANDRAD::DailyCycle::valueData_t>::const_iterator valueDataIT =
					std::find(dc->m_valueData.begin(), dc->m_valueData.end(), parameterName);
			IBK_ASSERT(valueDataIT != dc->m_valueData.end());
			const NANDRAD::DailyCycle::valueData_t & valData = *valueDataIT;

			// initialize interpolation type and check that interpolation type matches
			// (must be the same in all daily cycles)
			if (interpolationType == DailyCycle::NUM_IT)
				interpolationType = dc->m_interpolation;
			else if (interpolationType != dc->m_interpolation) {
				throw IBK::Exception(IBK::FormatString("Mismatching interpolation types for parameter '%1' in "
													   "different DailyCycles.").arg(parameterName), FUNC_ID);
			}

			// initialize unit and check that unit is always the same
			if (vals.m_unit.id() == 0) {
				vals.m_unit = valData.m_unit;
			}
			else {
				if (vals.m_unit != valData.m_unit)
					throw IBK::Exception(IBK::FormatString("Parameter '%1' was defined with unit '%2' and unit '%3' "
														   "in different DailyCycles. Please use the same unit for "
														   "the same parameter!")
										 .arg(parameterName).arg(vals.m_unit.name()).arg(valData.m_unit.name()), FUNC_ID);
			}


			// now finally append the daily spline to the global spline
			for (unsigned int h=0; h<dc->m_timePoints.size(); ++h) {
				double tpsec = dc->m_timePoints[h]*3600 + d*IBK::SECONDS_PER_DAY;
				double val = (*valData.m_valueVec)[h];
				if (tp.empty()) {
					tp.push_back(tpsec);
					vals.m_data.push_back(val);
				}
				else {
					// different handling for constant and linearly interpolated values
					if (interpolationType == NANDRAD::DailyCycle::IT_Constant) {
						// here we always add two points, one that ends the last interval and
						// one that starts the new interval
						tp.push_back(tpsec-TIME_SHIFT);
						vals.m_data.push_back(vals.m_data.back());
					}
					else {
						// check if same tp is already in the data vector, and by same we mean
						// dt = 0.1 s

						const double TIME_DELTA = 0.1;
						double tplast = tp.back();
						if (IBK::near_equal(tplast, tpsec, TIME_DELTA)) {
							// instead of adding the same time point, move the last time point backwards
							// Note: since we checked validity of all daily cycles already,
							//       we must have already at least 2 time points in the vector.
							IBK_ASSERT(tp.size() >= 2);
							// check if we can safely move the time point prior to
							tp.back() -= TIME_SHIFT;
						}
					}
					tp.push_back(tpsec);
					vals.m_data.push_back(val);
				}
			}
			dataAdded = true; // we have added data for the day
			break; // break the loop, since we have found our parameter set
		} // loop to process all cycles

		// throw an exception if a parameter definition is missing for a day
		if (!dataAdded) {
			throw IBK::Exception(IBK::FormatString("Couldn't find valid DailyCycle parameter data at day %1 "
												   "for parameter '%2'.").arg(d).arg(parameterName), FUNC_ID);
		}

	} // loop over all days of the year

	// in case of constant extrapolation, re-add the last time point close to the end of the interval
	// so that we can have cyclic use without error in the last interval
	if (interpolationType == NANDRAD::DailyCycle::IT_Constant) {
		tp.push_back(365*24*3600-TIME_SHIFT);
		vals.m_data.push_back(vals.m_data.back());
	}

	// convert the values to base SI unit and finally generate the spline
	vals.convert(IBK::Unit(vals.m_unit.base_id())); // does not throw
	// now generate spline
	spline.setValues(tp, vals.m_data); // does not throw

	/// \todo remove redundant values from spline, i.e when you have {0, 0; 5,0; 10,0} you can drop the middle point
	/// {0, 0; 10,0} without data loss
	std::string errmsg;
	bool success = spline.makeSpline(errmsg); // does not throw
	if (!success)
		throw IBK::Exception(IBK::FormatString("%1\nError creating spline for parameter '%2'. Possibly "
											   "a time interval ended too close to the end point of the interval. "
											   "Do not add time points < 24 h and less then < 2 minutes before the end of the "
											   "interval.").arg(errmsg).arg(parameterName), FUNC_ID);
}


bool Schedules::equalSchedules(const std::vector<Schedule> & first, const std::vector<Schedule> & second) {
	if(first.size() != second.size())
		return false;

	for(const NANDRAD::Schedule &schedA : first) {
		int counter = 0;
		for(const NANDRAD::Schedule &schedB : second) {
			if(schedA == schedB){
				++counter;
			}
		}
		if(counter != 1)
			return false;
	}

	return true;
}

bool Schedules::equalAnnualSchedules(const std::vector<LinearSplineParameter> &first, const std::vector<LinearSplineParameter> &second) {
	if(first.size() !=  second.size())
		return false;

	for(const NANDRAD::LinearSplineParameter &splA : first){
		int counter = 0;

		for(const NANDRAD::LinearSplineParameter &splB : second){
			if(splA == splB)
				++counter;
		}
		if(counter != 1)
			return false;
	}
	return true;
}


} // namespace NANDRAD

