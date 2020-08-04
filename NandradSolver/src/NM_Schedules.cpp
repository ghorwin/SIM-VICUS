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

#include "NM_ValueReference.h"

namespace NANDRAD_MODEL {


int Schedules::setTime(double t) {
	if (t == -1) return 0; // setTime wasn't called yet

	t += m_startTime;

	/// \todo think of handling multi-year splines vs. cyclic annual spline data. How do we distinguish that?
	/// Also in terms of interpolation method - actually, constant interpolation is not really nice and should
	/// by avoided at all cost - we need to store that extra information. IBK::LinearSpline has no storage members
	/// for that, so we may need an "extended" IBK::LinearSpline, one, that handles cyclic-year time clipping gracefully
	/// or simply holds a flag and we decided based on this flag, which time value to pass.

	// calculate all parameter values
	double * result = &m_results[0]; // points to first double in vector with calculated spline values
	for (int i = 0; i < NUM_R; ++i) {

		for (std::map<std::string, IBK::LinearSpline>::iterator it = m_scheduledQuantities[i].begin();
			 it != m_scheduledQuantities[i].end(); ++it)
		{
			*result = it->second.value(t);
			++result;
		}
	}
	return 0;
}


void Schedules::setup(const NANDRAD::Project &project) {
	/// \todo Anne

	// store start time offset as year and start time
	m_year = project.m_simulationParameter.m_intpara[NANDRAD::SimulationParameter::SIP_YEAR].value;
	m_startTime = project.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::IP_START].value;
}


double Schedules::startValue(const QuantityName &quantity) const {
	/// \todo why and what for is this needed?

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
}


const double * Schedules::resultValueRef(const ValueReference & quantityName) const {

	// search all results for current name
	/// \todo Anne

	return nullptr;
}


} // namespace NANDRAD_MODEL
