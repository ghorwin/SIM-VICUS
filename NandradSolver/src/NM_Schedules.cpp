/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
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
	// no results
	if (m_results.empty())
		return 0;

	// now t is moved to absolute time offset within start year
	t += m_startTime;

	// compute cyclic time, if we have cyclic use defined
	if (m_haveCyclicSchedules) {
		while (t > IBK::SECONDS_PER_YEAR)
			t -= IBK::SECONDS_PER_YEAR;
	}

	// calculate all parameter values
	double * result = &m_results[0]; // points to first double in vector with calculated spline values
	for (unsigned int i = 0; i<m_results.size(); ++i) {
		result[i] = m_valueSpline[i].value(t);
	}
	return 0;
}


void Schedules::setup(NANDRAD::Project &project) {
	FUNCID(Schedules::setup);
	// store start time offset as year and start time
	m_year = project.m_simulationParameter.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].value;
	m_startTime = project.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::P_Start].value;
	m_haveCyclicSchedules = project.m_schedules.m_flags[NANDRAD::Schedules::F_EnableCyclicSchedules].isEnabled();

	m_objectLists = &project.m_objectLists;
	m_schedules = &project.m_schedules;

	// run general input data checks
	project.m_schedules.checkParameters(project.m_placeholders);

	// loop over all daily cycle schedules and all linear spline schedules and remember the object lists
	for (std::map<std::string, std::vector<NANDRAD::Schedule> >::iterator schedGroupIT = project.m_schedules.m_scheduleGroups.begin();
		 schedGroupIT != project.m_schedules.m_scheduleGroups.end(); ++schedGroupIT)
	{
		std::vector<NANDRAD::Schedule> & schedules = schedGroupIT->second;
		// this set will contain list of all variables in this schedule group
		std::set< std::pair<std::string, IBK::Unit> > varlist;
		const std::string & objectListName = schedGroupIT->first;
		objectListByName(objectListName); // tests for existence and throws exception in case of missing schedule
		// now search through all schedules and collect list of variable names
		for (unsigned int i = 0; i < schedules.size(); ++i) {
			NANDRAD::Schedule & sched = schedules[i];
			try {
				sched.prepareCalculation();
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Error initializing schedule #%1 of "
														   "schedule group with object list '%2'.")
									 .arg(i+1).arg(objectListName), FUNC_ID);
			}
			// now we have a list of variables in this daily cycle - merge them with the global list of variables
			for (NANDRAD::DailyCycle & dc : sched.m_dailyCycles) {
				for (unsigned int i=0; i<dc.m_valueData.size(); ++i) {
					// Note: it is normal if several DailyCycles in various schedules define the same variables
					//       However, they should all be defined with the same SI unit
					for (auto p : varlist) {
						if (p.first == dc.m_valueData[i].m_name &&
							p.second != dc.m_valueData[i].m_unit)
						{
							throw IBK::Exception(IBK::FormatString("A daily cycle defines parameter '%1' with unit '%2', and "
																   "another daily cycle defines the same parameter with unit '%3'. "
																   "This is likely an error and must be fixed.")
								.arg(p.first).arg(p.second).arg(dc.m_valueData[i].m_unit), FUNC_ID);
						}
					}
					// remember this parameter-unit combination
					varlist.insert( std::make_pair(dc.m_valueData[i].m_name, dc.m_valueData[i].m_unit) );
				}
			}
		}
		// now we process all collect variables and store meta data about these variables
		for (auto var : varlist) {
			// check for correct spelling and unit of scheduled quantity
			KnownQuantities k;
			try {
				k = (KnownQuantities)NANDRAD_MODEL::KeywordList::Enumeration("Schedules::KnownQuantities", var.first);
			}
			catch (...) {
				throw IBK::Exception(IBK::FormatString("Unknown/undefined (perhaps misspelled) scheduled quantity '%1'.")
									 .arg(var.first), FUNC_ID);
			}
			// check unit
			IBK::Unit requestedUnit;
			try {
				requestedUnit = IBK::Unit(NANDRAD_MODEL::KeywordList::Unit("Schedules::KnownQuantities", k));
			} catch (...) {
				throw IBK::Exception(IBK::FormatString("ERROR: missing/invalid unit in 'Schedules::KnownQuantities' keyword list!"), FUNC_ID);
			}
			if (requestedUnit.base_id() != var.second.base_id())
				throw IBK::Exception(IBK::FormatString("Expected '%1' unit for scheduled quantity '%2', but "
													   "got unit with base SI-unit '%3' (not convertible).")
									 .arg(requestedUnit).arg(var.first).arg(var.second), FUNC_ID);

			m_variableNames.push_back(objectListName + "::" + var.first);
			m_variableUnits.push_back(var.second);
			// now generate the linear splines
			m_valueSpline.push_back(IBK::LinearSpline());
			// and initialize memory for corresponding result values
			m_results.push_back(0);

			// now populate spline
			IBK::LinearSpline & spl = m_valueSpline.back();

			NANDRAD::DailyCycle::interpolation_t interpolationType;
			try {
				m_schedules->generateLinearSpline(schedGroupIT->first, var.first, spl, interpolationType);
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, "Error initializing schedules (cannot generate schedule from daily cycle data).", FUNC_ID);
			}
		}
	}

	// setup annual splines
	for (auto itAnnualSched = m_schedules->m_annualSchedules.begin(); itAnnualSched!=m_schedules->m_annualSchedules.end(); ++itAnnualSched){
		// get object list name
		const std::string & objectListName = itAnnualSched->first;
		objectListByName(objectListName); // tests for existence and throws exception in case of missing schedule

		// iterate over all splines for that object list
		const std::vector<NANDRAD::LinearSplineParameter> &splines = itAnnualSched->second;
		for (const NANDRAD::LinearSplineParameter &spl: splines) {

			// NOTE: for the splines checkAndInitialize was called already, and referenced tsv-files have been read already

			// check for correct spelling and unit of scheduled quantity
			KnownQuantities k;
			try {
				k = (KnownQuantities)NANDRAD_MODEL::KeywordList::Enumeration("Schedules::KnownQuantities", spl.m_name);
			}
			catch (...) {
				throw IBK::Exception(IBK::FormatString("Unknown/undefined (perhaps misspelled) scheduled quantity '%1'.")
									 .arg(spl.m_name), FUNC_ID);
			}

			// check unit
			IBK::Unit requestedUnit;
			try {
				requestedUnit = IBK::Unit(NANDRAD_MODEL::KeywordList::Unit("Schedules::KnownQuantities", k));
			} catch (...) {
				throw IBK::Exception(IBK::FormatString("ERROR: missing/invalid unit in 'Schedules::KnownQuantities' keyword list!"), FUNC_ID);
			}
			if (requestedUnit.base_id() != spl.m_yUnit.base_id())
				throw IBK::Exception(IBK::FormatString("Expected '%1' unit for scheduled quantity '%2', but "
													   "got unit with base SI unit '%3' (not convertible).")
									 .arg(requestedUnit).arg(spl.m_name).arg(spl.m_yUnit), FUNC_ID);


			std::string varName = objectListName + "::" + spl.m_name;
			// do not allow duplicate annual schedules or mixed schedule group and annual schedule definitions
			// of the same quantity
			if(std::find(m_variableNames.begin(), m_variableNames.end(), varName) != m_variableNames.end())
				throw IBK::Exception(IBK::FormatString(	"Multiple definition of Quantity '%1' for ObjectList '%2' inside annual schedules. "
														"This is not allowed!")
									 .arg(spl.m_name).arg(objectListName), FUNC_ID);


			m_variableNames.push_back(varName);
			m_variableUnits.push_back(spl.m_yUnit);
			// now generate the linear splines
			m_valueSpline.push_back(spl.m_values);
			// and initialize memory for corresponding result values
			m_results.push_back(0);
		}
	}
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

	// if type is zone (MRT_ZONE), then do the following search:
	// - look for object lists that address zones
	//   - look in each object list, if id is part of the id group
	//     - in all remaining object lists, look if requested quantity is scheduled
	//       - if more than one match is found, throw an exception with "ambiguous schedule definition" error
	// finally, return memory address of searched quantity

	// similar for other schedules/reference types

	// collect all schedule group names
	std::set<std::string> scheduleGroupNames;
	for (auto schedGrp : m_schedules->m_scheduleGroups)
		scheduleGroupNames.insert(schedGrp.first);
	for (auto annualSched : m_schedules->m_annualSchedules)
		scheduleGroupNames.insert(annualSched.first);


	// find the object list that contains the requested object
	for (const std::string &schedName : scheduleGroupNames) {
		const NANDRAD::ObjectList * objList = objectListByName(schedName);
		IBK_ASSERT(objList != nullptr);
		std::string objectListName = objList->m_name;
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

				/// \todo performance enhancement: remember, which of the schedules have been requested,
				/// afterwards clear all linear splines that are unused and skip over these during model evaluation

				// populate quantity description, which is needed for output purposes and possible other models
				quantityDesc.m_name = valueRef.m_name.m_name;
				quantityDesc.m_size = 1;
				quantityDesc.m_unit = m_variableUnits[i].name();
				quantityDesc.m_constant = true;
				quantityDesc.m_description = "Schedule parameter: '" + valueRef.m_name.m_name + "'";

				return &m_results[i];
			}
		}
	}

	return nullptr;
}


void Schedules::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	std::string objectListName;
	// loop through all variables and create a suitable result description
	for (unsigned int i=0; i<m_variableNames.size(); ++i) {
		// spilt into object list name and quantity name
		size_t pos = m_variableNames[i].find("::");
		IBK_ASSERT(pos != std::string::npos);
		// name
		const std::string &objectListName = m_variableNames[i].substr(0,pos);
		const std::string &quantityName = m_variableNames[i].substr(pos + 2);
		// find object list
		const NANDRAD::ObjectList * objList = objectListByName(objectListName);
		// skip empty object lists
		if(objList->m_filterID.m_ids.empty())
			continue;
		// create a result description template
		QuantityDescription quantityDesc;
		quantityDesc.m_referenceType = objList->m_referenceType;
		quantityDesc.m_name = quantityName;
		quantityDesc.m_unit = m_variableUnits[i].name();
		quantityDesc.m_size = 1; // we never have vector-valued schedule quantities
		quantityDesc.m_description = "Schedule parameter: '" + quantityName + "'";
		quantityDesc.m_constant = true;

		// object lists are resolved, already
		// therefore, loop through all registered ids and create a quantity
		// description
		for(unsigned int id : objList->m_filterID.m_ids) {
			// create an id entry for each quantity
			quantityDesc.m_id = id;
			// add to container
			resDesc.push_back(quantityDesc);
		}
	}
}


} // namespace NANDRAD_MODEL
