/*	The Nandrad model library.

Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
All rights reserved.

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

#include <NANDRAD_DailyCycle.h>
#include <NANDRAD_Interval.h>
#include <NANDRAD_KeywordList.h>
#include <NANDRAD_LinearSplineParameter.h>
#include <NANDRAD_Project.h>
#include <NANDRAD_Schedule.h>
#include <NANDRAD_SimulationParameter.h>

using namespace std;

namespace NANDRAD_MODEL {


int Schedules::setTime(double t) {
#if 0
	if (t == -1) return 0; // setTime wasn't called yet
	IBK_ASSERT(m_startTime != NULL);
	// caluclate all parameter values
	for(unsigned int i = 0; i < m_scheduleParameters.size(); ++i)
	{
		m_results[i].value = m_scheduleParameters[i].value(*m_startTime + t);
	}
	for(unsigned int i = 0; i < m_annualScheduleParameters.size(); ++i)
	{
		if (m_annualScheduleParameters[i].m_interpolationMethod == NANDRAD::LinearSplineParameter::I_Constant) {
			m_results[i + m_scheduleParameters.size()].value =
				m_annualScheduleParameters[i].m_values.nonInterpolatedValue(*m_startTime + t);
		}
		else {
			m_results[i + m_scheduleParameters.size()].value =
				m_annualScheduleParameters[i].m_values.value(*m_startTime + t);
		}
	}
	// signal success
#endif
	return 0;
}


void Schedules::setup(const NANDRAD::Project &project) {
#if 0
	const char * const FUNC_ID = "[Schedules::setup]";
	// empty schedules
	if(project.m_schedules.m_scheduleGroups.empty() )
		return;

	// store project pointer
	m_project = &project;
	// setup time definition
	m_scheduleDays.init(project.m_schedules, project.m_simulationParameter);

	// retreive start time offset
	IBK_ASSERT(!project.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::IP_START].name.empty());
	m_startTime = &project.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::IP_START].value;

	// now initialise all scheduled quantities
	std::map<std::string, IBK::Unit> quantities;
	// loop over all schedule groups
	for (unsigned int i=0; i< project.m_schedules.m_scheduleGroups.size(); ++i) {
		const NANDRAD::ScheduleGroup &scheduleGroup = project.m_schedules.m_scheduleGroups[i];
		// loop over all space type definitions
		std::map<std::string, NANDRAD::ScheduleGroup::ScheduleMap>::const_iterator spaceTypeIt =
			scheduleGroup.m_spaceTypeGroups.begin();
		for ( ; spaceTypeIt != scheduleGroup.m_spaceTypeGroups.end(); ++spaceTypeIt) {
			// loop over all schedule definitions
			NANDRAD::ScheduleGroup::ScheduleMap::const_iterator scheduleIt =
				spaceTypeIt->second.begin();
			for( ; scheduleIt != spaceTypeIt->second.end(); ++scheduleIt)
				scheduleIt->second.collectQuantities(spaceTypeIt->first, quantities);
		}
	}

	// we have as many variables as quantities defined within the schedules block
	m_results.clear();

	std::set<std::string> definedParameters;
	// generate variable references that can be referenced as ModelInputReference by models and outputs
	for (std::map<std::string, IBK::Unit>::const_iterator it = quantities.begin(); it != quantities.end(); ++it) {
		// ensure that we find a suitable key
		std::string::size_type pos = it->first.rfind(':');
		IBK_ASSERT(pos != std::string::npos);
		std::string keyName = it->first.substr(pos + 1);
		QuantityName keyTarget;
		keyTarget.fromEncodedString(keyName);
		std::string spaceTypeName = it->first.substr(0, pos);

		// error: undefined schedule quantity
		if (!KeywordList::KeywordExists("Schedules::Results", keyTarget.name())) {
			throw IBK::Exception(IBK::FormatString("Undefined schedule parameter #%1 inside space type group #%2!")
				.arg(keyName)
				.arg(spaceTypeName), FUNC_ID);
		}

		Results resEnum = (Results) KeywordList::Enumeration("Schedules::Results", keyTarget.name());
		std::string keyUnit = KeywordList::Unit("Schedules::Results", resEnum);

		// error: wrong unit
		IBK::Parameter paraConvert(keyName, 0, keyUnit);
		if (it->second.name() != keyUnit) {
			try {
				paraConvert.get_value(it->second.name());
			}
			catch (IBK::Exception &ex) {
				throw IBK::Exception(ex, IBK::FormatString("Invalid unit '#%1' of schedule parameter #%2 inside "
					"space type group #%3!")
					.arg(it->second.name())
					.arg(keyName)
					.arg(spaceTypeName), FUNC_ID);
			}
		}

		// treat all values as scalar quantities
		m_results.push_back(IBK::Parameter(it->first, 0, it->second));
		// store parameter name
		definedParameters.insert(it->first);
	}

	std::map<std::string, NANDRAD::AnnualSchedules::LinearSplineParameterMap>::const_iterator spaceTypeIt =
		project.m_schedules.m_annualSchedules.m_parameters.begin();
	// first fill parameters with annual schedules
	for( ; spaceTypeIt != project.m_schedules.m_annualSchedules.m_parameters.end(); ++spaceTypeIt)
	{
		const std::string spaceTypeName = spaceTypeIt->first;
		NANDRAD::AnnualSchedules::LinearSplineParameterMap::const_iterator  paraIt = spaceTypeIt->second.begin();
		// loop over all annual schedule parameters
		for( ; paraIt != spaceTypeIt->second.end(); ++paraIt)
		{
			const std::string paraName = spaceTypeName + std::string(":") + paraIt->first;
			// check if any parameter is defined twice
			if(definedParameters.find(paraName) != definedParameters.end())
				throw IBK::Exception(IBK::FormatString("Error defining schedule parameter %1: "
														"More than one annual schedule parameter are defined.")
														.arg(paraName),
														FUNC_ID);
			// retrieve parameter unit
			const IBK::Unit &paraUnit = paraIt->second.m_yUnit;
			m_results.push_back( IBK::Parameter(paraName, 0, paraUnit) );
			definedParameters.insert(paraName);
		}
	}

	// construct schedule parameter
	m_scheduleParameters.clear();

	for (std::map<std::string, IBK::Unit>::const_iterator it = quantities.begin(); it != quantities.end(); ++it) {
		// retrieve current Quantity
		const std::string quantity = it->first;
		// construct parameter using name, time unit, value unit
		m_scheduleParameters.push_back( ScheduleParameter(it->first, "s", it->second.name(), m_scheduleDays ) );
	}

	// fill schedule parameter
	for(unsigned int s = 0; s < m_scheduleParameters.size(); ++s)
	{
		ScheduleParameter &scheduleParameter = m_scheduleParameters[s];
		// add groups to schedule parameter: reverse order
		for(unsigned int i = (unsigned int) project.m_schedules.m_scheduleGroups.size(); i > 0 ; --i)
		{
			const NANDRAD::ScheduleGroup &scheduleGroup =
				project.m_schedules.m_scheduleGroups[i - 1];
			// basic schedule group must! include all schedule parameters
			if (scheduleGroup.m_startDate.m_name.empty() &&
				scheduleGroup.m_endDate.m_name.empty()) {

				scheduleParameter.addScheduleGroup(scheduleGroup);
			}
			// otherwise only add schedule group that contain current space type group
			else {
				// split name into space type and quantity name
				std::string spaceTypeName;
				std::vector<std::string> tokens;
				// split quantity name into spaceType name and quantity
				IBK::explode_in2(scheduleParameter.m_name, tokens, ':');
				spaceTypeName = tokens[0];
				// only add valid groups
				if (scheduleGroup.m_spaceTypeGroups.find(spaceTypeName) !=
					scheduleGroup.m_spaceTypeGroups.end())
				{
					scheduleParameter.addScheduleGroup(scheduleGroup);
				}
			}
		}
		// check if schedule parameter is valid
		std::string errmsg;
		if(!scheduleParameter.valid(errmsg))
				throw IBK::Exception(IBK::FormatString("Error initializing schedule parameter %1: %2.")
									.arg(scheduleParameter.m_name)
									.arg(errmsg),
									FUNC_ID);
	}

	// construct annual schedule parameters
	m_annualScheduleParameters.clear();

	std::map<std::string, NANDRAD::AnnualSchedules::LinearSplineParameterMap>::const_iterator annschedIt =
		project.m_schedules.m_annualSchedules.m_parameters.begin();

	for ( ;	annschedIt != project.m_schedules.m_annualSchedules.m_parameters.end();
		++annschedIt) {
		// retrieve space type name
		const std::string &spaceTypeName = annschedIt->first;

		NANDRAD::AnnualSchedules::LinearSplineParameterMap::const_iterator paraIt
			= annschedIt->second.begin();
		// retrieve current Quantity
		for( ; paraIt != annschedIt->second.end(); ++paraIt)
		{
			std::string keyName = paraIt->second.m_name;
			// error: undefined schedule quantity
			if (!KeywordList::KeywordExists("Schedules::Results", keyName)) {
				throw IBK::Exception(IBK::FormatString("Undefined annual schedule parameter #%1!")
					.arg(keyName), FUNC_ID);
			}

			Results resEnum = (Results)KeywordList::Enumeration("Schedules::Results", keyName);
			std::string keyUnit = KeywordList::Unit("Schedules::Results", resEnum);

			// error: wrong unit
			IBK::Parameter paraConvert(keyName, 0, keyUnit);
			if (paraIt->second.m_yUnit.name() != keyUnit) {
				try {
					paraConvert.get_value(paraIt->second.m_yUnit);
				}
				catch (IBK::Exception &ex) {
					throw IBK::Exception(ex, IBK::FormatString("Invalid unit '#%1' of annual schedule parameter #%2!")
						.arg(paraIt->second.m_yUnit)
						.arg(keyName), FUNC_ID);
				}
			}

			NANDRAD::LinearSplineParameter splineParameter;
			// construct scehdule parameter name from space type and parameter name
			std::string name = spaceTypeName + std::string(":") + paraIt->second.m_name;
			splineParameter.m_name = name;
			splineParameter.m_xUnit = paraIt->second.m_xUnit;
			splineParameter.m_yUnit = paraIt->second.m_yUnit;

			// copy values
			splineParameter.m_values = paraIt->second.m_values;
			std::string errmsg;
			splineParameter.m_values.makeSpline(errmsg);
			if(! errmsg.empty() )
				throw IBK::Exception(IBK::FormatString("Error creating annual schedule parameter %1: %2")
				.arg(name)
				.arg(errmsg),
				FUNC_ID);

			// construct linear spline
			// fill annual schedule parameter vector
			m_annualScheduleParameters.push_back(splineParameter);
		}
	}
#endif
}


void Schedules::initResults(const std::vector<AbstractModel*> &  models) {
#if 0
	const char *const FUNC_ID = "[Schedules::initResults]";

	// use result descriptions for schedule parameter checking
	std::vector<QuantityDescription> resDescs;
	resultDescriptions(resDescs);

	// now check if all schedule parameters are inside defined interval
	for (unsigned int i = 0; i < m_scheduleParameters.size(); ++i) {

		const ScheduleParameter &scheduleParameter = m_scheduleParameters[i];

		// check if we have constraints
		std::vector<QuantityDescription>::const_iterator resDescIt
			= std::find_if(resDescs.begin(), resDescs.end(),
				NANDRAD::FindByName<QuantityDescription>(scheduleParameter.m_name) );

		// ignore parameters without constraints definition
		IBK_ASSERT(resDescIt != resDescs.end());
		const double minVal = resDescIt->m_minMaxValue.first;
		const double maxVal = resDescIt->m_minMaxValue.second;

		// check validity of schedule interval: first base schedule
		for (std::map<int, ScheduleParameter::singleSchedulePameter>::const_iterator
			it = scheduleParameter.m_basicScheduleGroupParameter.begin();
			it != scheduleParameter.m_basicScheduleGroupParameter.end();
			++it) {
			// get spline for daily paarmeter definition
			const IBK::LinearSpline &dailySpline = it->second.second;
			// check all values
			for (unsigned int k = 0; k < dailySpline.y().size(); ++k) {
				const double value = dailySpline.y()[k];
				// check if value meets constraints and throw anexception
				// otherwise
				if (value < minVal) {
					throw IBK::Exception(IBK::FormatString("Error in schedule definition: "
						"Quantity '#%1' with #%2 #%3 "
						"is smaller than the allowed minimum value #%4!")
						.arg(scheduleParameter.m_name)
						.arg(value)
						.arg(IBK::Unit(scheduleParameter.m_yUnit).base_unit())
						.arg(minVal), FUNC_ID);
				}
				if (value > maxVal) {
					throw IBK::Exception(IBK::FormatString("Error in schedule definition: "
						"Quantity '#%1' with #%2 #%3 "
						"is larger than the allowed maximum value #%4!")
						.arg(scheduleParameter.m_name)
						.arg(value)
						.arg(IBK::Unit(scheduleParameter.m_yUnit).base_unit())
						.arg(maxVal), FUNC_ID);
				}
			}
		}

		for (unsigned int j = 0; j < scheduleParameter.m_specialScheduleGroupParameters.size(); ++j) {
			// now check validity of schedule for special schedule group parameters
			for (std::map<int, ScheduleParameter::singleSchedulePameter>::const_iterator
				it = scheduleParameter.m_specialScheduleGroupParameters[j].begin();
				it != scheduleParameter.m_specialScheduleGroupParameters[j].end();
				++it) {
				// get spline for daily paarmeter definition
				const IBK::LinearSpline &dailySpline = it->second.second;
				// check all values
				for (unsigned int k = 0; k < dailySpline.y().size(); ++k) {
					const double value = dailySpline.y()[k];
					// check if value meets constraints and throw anexception
					// otherwise
					if (value < minVal) {
						throw IBK::Exception(IBK::FormatString("Error in schedule definition: "
							"Quantity '#%1' with #%2 #%3 "
							"is smaller than the allowed minimum value #%4!")
							.arg(scheduleParameter.m_name)
							.arg(value)
							.arg(IBK::Unit(scheduleParameter.m_yUnit).base_unit())
							.arg(minVal), FUNC_ID);
					}
					if (value > maxVal) {
						throw IBK::Exception(IBK::FormatString("Error inschedule definition: "
							"Quantity '#%1' with #%2 #%3 "
							"is larger than the allowed maximum value #%4!")
							.arg(scheduleParameter.m_name)
							.arg(value)
							.arg(IBK::Unit(scheduleParameter.m_yUnit).base_unit())
							.arg(maxVal), FUNC_ID);
					}
				}
			}
		}
	}
	// now check if all annual schedule parameters are inside defined interval
	for (unsigned int i = 0; i < m_annualScheduleParameters.size(); ++i) {
		const NANDRAD::LinearSplineParameter &scheduleParameter = m_annualScheduleParameters[i];

		// check if we have constraints
		std::vector<QuantityDescription>::const_iterator resDescIt
			= std::find_if(resDescs.begin(), resDescs.end(),
				NANDRAD::FindByName<QuantityDescription>(scheduleParameter.m_name));

		// ignore parameters without constraints definition
		IBK_ASSERT(resDescIt != resDescs.end());
		const double minVal = resDescIt->m_minMaxValue.first;
		const double maxVal = resDescIt->m_minMaxValue.second;

		// check all values
		for (unsigned int k = 0; k < scheduleParameter.m_values.y().size(); ++k) {
			const double value = scheduleParameter.m_values.y()[k];
			// check if value meets constraints and throw anexception
			// otherwise
			if (value < minVal) {
				throw IBK::Exception(IBK::FormatString("Error in schedule definition: "
					"Quantity '#%1' with #%2 #%3 "
					"is smaller than the allowed minimum value #%4!")
					.arg(scheduleParameter.m_name)
					.arg(value)
					.arg(IBK::Unit(scheduleParameter.m_yUnit).base_unit())
					.arg(minVal), FUNC_ID);
			}
			if (value > maxVal) {
				throw IBK::Exception(IBK::FormatString("Error in schedule definition: "
					"Quantity '#%1' with #%2 #%3 "
					"is larger than the allowed maximum value #%4!")
					.arg(scheduleParameter.m_name)
					.arg(value)
					.arg(IBK::Unit(scheduleParameter.m_yUnit).base_unit())
					.arg(maxVal), FUNC_ID);
			}
		}
	}
	// set start values
	setTime(0);
#endif
}

double Schedules::startValue(const QuantityName &quantity) const {
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
		IBK_ASSERT(m_startTime != NULL);
		return paraIt->value(*m_startTime);
	}
	// find quantity name in annual schedules
	std::vector<NANDRAD::LinearSplineParameter>::const_iterator splineParaIt
		= std::find_if(m_annualScheduleParameters.begin(),
			m_annualScheduleParameters.end(),
			NANDRAD::FindByName<NANDRAD::LinearSplineParameter>(quantityName));
	// found in annual schedule parameters
	if (splineParaIt != m_annualScheduleParameters.end()) {
		IBK_ASSERT(m_startTime != NULL);
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
}

void Schedules::resultValueRefs(std::vector<const double *> &res) const {

	res.clear();
	// fill with all results

	for (unsigned int i = 0; i < m_results.size(); ++i) {
		res.push_back(&m_results[i].value);
	}
}


const double * Schedules::resultValueRef(const QuantityName & quantityName) const {

	// search all results for current name
	for (unsigned int i = 0; i < m_results.size(); ++i) {
		if (m_results[i].name == quantityName.name())
			return &m_results[i].value;
	}

	return NULL;
}


void Schedules::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {

	QuantityDescription result;

	for (unsigned int i = 0; i<m_results.size(); ++i) {

		// create a new result description
		result.m_name = m_results[i].name;
		result.m_unit = m_results[i].IO_unit.name();
		result.m_size = 1;

		// reduce target name by spacxe type specification
		std::string keyname = m_results[i].name;
		std::string::size_type pos = keyname.rfind(':');
		IBK_ASSERT(pos != std::string::npos);
		keyname = keyname.substr(pos + 1);

		// reduce target name by index notation
		QuantityName keyTarget;
		keyTarget.fromEncodedString(keyname);

		// check vaidity of target
		IBK_ASSERT(KeywordList::KeywordExists("Schedules::Results", keyTarget.name()));
		Results resEnum = (Results) KeywordList::Enumeration("Schedules::Results", keyTarget.name());
		// copy description
		result.m_description = KeywordList::Description("Schedules::Results", resEnum);

		// check minimum and maximum values
		switch (resEnum) {
			// *** heating set point temperature >= -100°C ***
			case R_HeatingSetPointTemperature:
				result.m_minMaxValue = std::make_pair(173.15, std::numeric_limits<double>::max());
			break;
			// *** cooling set point temperature >= -100°C ***
			case R_CoolingSetPointTemperature:
				result.m_minMaxValue = std::make_pair(173.15, std::numeric_limits<double>::max());
			break;
			// *** air condition set point temperature >= -100°C ***
			case R_AirConditionSetPointTemperature:
				result.m_minMaxValue = std::make_pair(173.15, std::numeric_limits<double>::max());
			break;
			// *** air condition set point mass flux >= 0 ***
			case R_AirConditionSetPointMassFlux:
				result.m_minMaxValue = std::make_pair(0, std::numeric_limits<double>::max());
				break;
			// *** air condition set point relative humidity in [0,1] ***
			case R_AirConditionSetPointRelativeHumidity:
				result.m_minMaxValue = std::make_pair(0.0, 1.0);
			break;
			// *** heating load >= 0
			case R_HeatingLoad:
				result.m_minMaxValue = std::make_pair(0.0, std::numeric_limits<double>::max());
			break;
			// *** cooling power >= 0
			case R_CoolingPower:
				result.m_minMaxValue = std::make_pair(0.0, std::numeric_limits<double>::max());
			break;
			// *** lighting power >= 0
			case R_LightingPower:
				result.m_minMaxValue = std::make_pair(0.0, std::numeric_limits<double>::max());
			break;
			// *** thermal energy loss per person >= 0
			case R_ThermalEnergyLossPerPerson:
				result.m_minMaxValue = std::make_pair(0.0, std::numeric_limits<double>::max());
			break;
			// *** total energy production per person >=0 ***
			case R_TotalEnergyProductionPerPerson:
				result.m_minMaxValue = std::make_pair(0.0, std::numeric_limits<double>::max());
			break;
			// *** total moisture release per person >=0 ***
			case R_MoistureReleasePerPerson:
				result.m_minMaxValue = std::make_pair(0.0, std::numeric_limits<double>::max());
			break;
			// *** total CO2 emission per person >=0 ***
			case R_CO2EmissionPerPerson:
				result.m_minMaxValue = std::make_pair(0.0, std::numeric_limits<double>::max());
			break;
			// *** mass flux rate in [0,1] ***
			case R_MassFluxRate:
				result.m_minMaxValue = std::make_pair(0.0, 1.0);
			break;
			// *** pressure head is larger than 0 ***
			case R_PressureHead:
				result.m_minMaxValue = std::make_pair(0.0, std::numeric_limits<double>::max());
			break;
			// *** occupancy rate in [0,1] ***
			case R_OccupancyRate:
				result.m_minMaxValue = std::make_pair(0.0, 1.0);
			break;
			// *** equipment utilization ratio in [0,1] ***
			case R_EquipmentUtilizationRatio:
				result.m_minMaxValue = std::make_pair(0.0, 1.0);
			break;
			// *** lighting utilization ratio in [0,1] ***
			case R_LightingUtilizationRatio:
				result.m_minMaxValue = std::make_pair(0.0, 1.0);
			break;
			// *** ventilation air change rate >= 0 ***
			case R_UserVentilationAirChangeRate:
				result.m_minMaxValue = std::make_pair(0.0, std::numeric_limits<double>::max());
			break;
			// *** infiltratiom air change rate >= 0 ***
			case R_InfiltrationAirChangeRate:
				result.m_minMaxValue = std::make_pair(0.0, std::numeric_limits<double>::max());
			break;
			// *** shading factor in [0,1] ***
			case R_ShadingFactor:
				result.m_minMaxValue = std::make_pair(0.0, 1.0);
				break;
			default:
			break;
		}

		resDesc.push_back(result);
		// clear result
		result.m_minMaxValue = std::make_pair(-std::numeric_limits<double>::max(),
			std::numeric_limits<double>::max());
	}
}

//const NANDRAD::Schedules *Schedules::schedules() const {
//	if (m_project != NULL)
//		return &m_project->m_schedules;
//	return NULL;
//}


} // namespace NANDRAD_MODEL
