/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#include "NM_Schedules.h"

#include "NM_AbstractStateDependency.h"
#include "NM_KeywordList.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <algorithm>

#include <IBK_assert.h>
#include <IBK_FileUtils.h>
#include <IBK_StringUtils.h>
#include <IBK_Time.h>
#include <IBK_Constants.h>

#include <NANDRAD_DailyCycle.h>
#include <NANDRAD_Interval.h>
#include <NANDRAD_KeywordList.h>
#include <NANDRAD_LinearSplineParameter.h>
#include <NANDRAD_Project.h>
#include <NANDRAD_Schedule.h>
#include <NANDRAD_Schedules.h>
#include <NANDRAD_SimulationParameter.h>

#include "NM_InputReference.h"

namespace NANDRAD_MODEL {


int Schedules::setTime(double t) {
	if (t == -1)
		return 0; // initialization call, cannot calculate
	// no results
	if (m_results.empty())
		return 0;

	/// \todo clarify cyclic vs. continuous handling, especially alignment of weekdays/weekends
	///       Suppose we have a cyclic schedule, but change start year from 2008 to 2009, we will then have
	///       different day types at the same date. This might be desirable, or annoying... discuss!

	// now t is moved to absolute time offset within start year
	t += m_startTime;

	// compute cyclic time
	double t_cyclic = t;
	while (t_cyclic > IBK::SECONDS_PER_YEAR)
		t_cyclic -= IBK::SECONDS_PER_YEAR;

	/// \todo think about week cycle time, so that we have t_weekly running from t_weekly=0 -> start of monday
	///       then we can work with weekly data

	// calculate all parameter values
	double * result = &m_results[0]; // points to first double in vector with calculated spline values
	for (int i = 0; i < NUM_R; ++i) {

		for (std::map<std::string, NANDRAD::LinearSplineParameter>::iterator it = m_scheduledQuantities[i].begin();
			 it != m_scheduledQuantities[i].end(); ++it)
		{
			NANDRAD::LinearSplineParameter & p = it->second;
			// depending on time cycling value, pass either t or t_cyclic
			if (p.m_wrapMethod == NANDRAD::LinearSplineParameter::C_CYCLIC)
				*result = p.m_values.value(t_cyclic);
			else
				*result = p.m_values.value(t);
			// move memory slot forward
			++result;
		}
	}
	return 0;
}


void Schedules::setup(const NANDRAD::Project &project) {
	FUNCID(Schedules::setup);
	// store start time offset as year and start time
	m_year = project.m_simulationParameter.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].value;
	m_startTime = project.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::P_Start].value;

	m_objectLists = &project.m_objectLists;
	m_schedules = &project.m_schedules;

	// this set will contain list of all variables ('objectlist::varname')
	std::set<std::string> varlist;

	// loop over all daily cycle schedules and all linear spline schedules and remember the object lists
	for (auto schedGroup : project.m_schedules.m_scheduleGroups) {
		const std::string & objectListName = schedGroup.first;
		objectListByName(objectListName); // tests for existence and throws exception in case of missing schedule
		// now search through all schedules and collect list of variable names
		for (unsigned int i = 0; i < schedGroup.second.size(); ++i) {
			NANDRAD::Schedule & sched = schedGroup.second[i];
			// loop over all daily cycles and initialize them
			for (NANDRAD::DailyCycle & dc : sched.m_dailyCycles) {
				try {
					dc.prepareCalculation();
				} catch (IBK::Exception & ex) {
					throw IBK::Exception(ex, IBK::FormatString("Error initializing DailyCycle in schedule #%1 of "
															   "schedule group with object list '%2'.")
										 .arg(i+1).arg(objectListName), FUNC_ID);

				}
			}

		}
	}
}


double Schedules::startValue(const QuantityName &quantity) const {
	/// \todo why and what for is this needed?
	/// Normally, calling setTime(0) should initialize outputs with the start value, which can then be retrieved as
	/// usual.

#if 0
	const char* const FUNC_ID = "[Schedules::startValue]";
	// create quantityname
	std::string quantityName = quantity.name();
	// additional index
	if (quantity.index() != -1)
	{
		quantityName += std::string("[id=") + IBK::val2string<unsigned int>(quantity.index())
			+ std::string("]");
	}
	// find quantity name in scheudle parameter
	std::vector<ScheduleParameter>::const_iterator paraIt
		= std::find_if(m_scheduleParameters.begin(),
			m_scheduleParameters.end(),
			NANDRAD::FindByName<ScheduleParameter>(quantityName));

	// found in schedule parameters
	if (paraIt != m_scheduleParameters.end()) {
		IBK_ASSERT(m_startTime != nullptr);
		return paraIt->value(*m_startTime);
	}
	// find quantity name in annual schedules
	std::vector<NANDRAD::LinearSplineParameter>::const_iterator splineParaIt
		= std::find_if(m_annualScheduleParameters.begin(),
			m_annualScheduleParameters.end(),
			NANDRAD::FindByName<NANDRAD::LinearSplineParameter>(quantityName));
	// found in annual schedule parameters
	if (splineParaIt != m_annualScheduleParameters.end()) {
		IBK_ASSERT(m_startTime != nullptr);
		if (splineParaIt->m_interpolationMethod == NANDRAD::LinearSplineParameter::I_Constant) {
			return splineParaIt->m_values.nonInterpolatedValue(*m_startTime);
		}
		else {
			return splineParaIt->m_values.value(*m_startTime);
		}
	}

	// not defined
	throw IBK::Exception(IBK::FormatString("Error retrieving start value for quantity '%1' " "from schedules! Parameter is undefined!")
		.arg(quantityName), FUNC_ID);
#endif
	return 0;
}


const NANDRAD::ObjectList * Schedules::objectListByName(const std::string & objectListName) const {
	FUNCID(Schedules::objectListByName);
	// find this object list
	std::vector<NANDRAD::ObjectList>::const_iterator it = std::find(m_objectLists->begin(),
																	m_objectLists->end(),
																	objectListName);
	if (it == m_objectLists->end())
		throw IBK::Exception( IBK::FormatString("Object list '%1' referenced from schedule group is not defined.")
							  .arg(objectListName), FUNC_ID);
	return &(*it);
}


const double * Schedules::resolveResultReference(const InputReference & valueRef, QuantityDescription & quantityDesc) const {
	FUNCID(Schedules::resolveResultReference);

	// variable lookup rules:

	// if type is an object list (MRT_OBJECTLIST), then search for quantity within stored object lists maps:
	// - search until requested quantity is found
	//   - if more than one match is found, throw an exception with "ambiguous schedule definition" error
	//     (should not be possible, since no two schedules may be parametrized for the same object list - would
	//     already fail during reading
	//
	// if type is zone (MRT_ZONE), then do the following search:
	// - look for object lists that address zones
	//   - look in each object list, if id is part of the id group
	//     - in all remaining object lists, look if requested quantity is scheduled
	//       - if more than one match is found, throw an exception with "ambiguous schedule definition" error
	// finally, return memory address of searched quantity

	// similar for other schedules/reference types

	std::string objectListName;
	if (valueRef.m_referenceType == NANDRAD::ModelInputReference::MRT_OBJECTLIST) {
		// quantity name is composed of
		//objectListName
	}
	else {
		// find the object list that contains the requested object
		// first search the schedule groups
		for (auto schedGrp : m_schedules->m_scheduleGroups) {
			const NANDRAD::ObjectList * objList = objectListByName(schedGrp.first);
			IBK_ASSERT(objList != nullptr);
			// correct reference type?
			if (objList->m_referenceType != valueRef.m_referenceType)
				continue; // not our input reference
			// id range correct
			if (!objList->m_filterID.contains(valueRef.m_id))
				continue; // not our input reference
			// search through results to find value
			std::string valueName = objectListName + "::" + valueRef.m_name.m_name;
			for (unsigned int i=0; i<m_variableNames.size(); ++i) {
				if (m_variableNames[i] == valueName) {
					// found the variable name - check that user did not (accidentally) request
					// a vector-valued quantity
					if (valueRef.m_name.m_index != -1)
						throw IBK::Exception(IBK::FormatString("Vector-valued quantity '%1' matches a scheduled scalar "
															   "parameter. This is an invalid reference.")
											 .arg(valueRef.m_name.m_name), FUNC_ID);
					return &m_results[i];
				}
			}
		}
	}

	return nullptr;
}


} // namespace NANDRAD_MODEL
