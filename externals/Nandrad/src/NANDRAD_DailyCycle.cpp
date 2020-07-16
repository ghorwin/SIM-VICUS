/*	The NANDRAD data model library.
Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
St. Vogelsang	<stefan.vogelsang -[at]- tu-dresden.de>
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

#include "NANDRAD_Constants.h"
#include "NANDRAD_DailyCycle.h"
#include "NANDRAD_KeywordList.h"

namespace NANDRAD {

void DailyCycle::createLinearSpline(const std::string &quantityName, IBK::LinearSpline &spline) const {
#if 0
	const char * const FUNC_ID = "[DailyCycle::linearSpline]";

	spline.clear();
	// check if interval section contains quantity name
	std::string quantityUnit;
	unsigned int jFirst = 0;
	bool intervalsContainQuantity = false, hourlyValuesContainQuantity = false;
	for (unsigned int i = 0; i < m_intervals.size(); ++i) {
		// we found quantity inside interval definition
		if(m_intervals[i].m_genericParaConst.find(quantityName)
			!= m_intervals[i].m_genericParaConst.end()) {
			intervalsContainQuantity = true;
			break;
		}
	}
	// check if hourly values section contains quantity name
	for ( jFirst = 0; jFirst < m_hourlyValues.size(); ++jFirst)
	{
		if(m_hourlyValues[jFirst].m_name != quantityName)
			continue;

		hourlyValuesContainQuantity = true;
		break;
	}
	// no schedule definition for the requested quantity, return an empty spline parameter
	if(!hourlyValuesContainQuantity && !intervalsContainQuantity)
		return;
	// error: double definition
	if(hourlyValuesContainQuantity && intervalsContainQuantity)
			throw IBK::Exception(IBK::FormatString("Error converting daily cycle definition to a linear spline for quantity with name %1: "
								"Both a schedule and an interval definition are given, the definition is not unique.")
								.arg(quantityName),
								FUNC_ID);
	// fill parameter
	if(intervalsContainQuantity)
	{
		// check validity of interval definition
		try {
			checkIntervalDefinition();
		}
		catch(IBK::Exception &ex)
		{
			throw IBK::Exception(ex, IBK::FormatString("Error converting daily cycle definition to a linear spline for quantity with name %1.")
								.arg(quantityName),
								FUNC_ID);
		}
		// retreive the quantity unit
		std::map<std::string, IBK::Parameter>::const_iterator
					firstParaIt = m_intervals[0].m_genericParaConst.find(quantityName);
		// iterator must not be empty
		if(firstParaIt == m_intervals[0].m_genericParaConst.end()) {
			throw IBK::Exception(IBK::FormatString("Error converting interval definition to a linear spline for quantity with name %1: "
							"The quantity value is not defined inside the interval %2.")
							.arg(quantityName)
							.arg(0),
							FUNC_ID);
		}
		// retreive parameter
		quantityUnit = firstParaIt->second.unit().name();
		// retreive time definition and parameter values
		std::vector<double> xValues(1,0);
		std::vector<double> yValues;
		// loop through all intervals
		for(unsigned int i = 0; i < m_intervals.size(); ++i)
		{
			const Interval &interval = m_intervals[i];
			// check if interval contain a matching parameter
			std::map<std::string, IBK::Parameter>::const_iterator
			paraIt = interval.m_genericParaConst.find(quantityName);
			// error: parameter requested from first interval is not found inside the other intervals
			if(paraIt == interval.m_genericParaConst.end() ) {
					throw IBK::Exception(IBK::FormatString("Error converting interval definition to a linear spline for quantity with name %1: "
									"The quantity value is not defined inside the interval %2.")
									.arg(quantityName)
									.arg(i),
									FUNC_ID);
			}
			IBK_ASSERT(!interval.m_para[Interval::IP_END].name.empty()
				|| i == m_intervals.size() - 1);
			// time definition by 'end'
			if(!interval.m_para[Interval::IP_END].name.empty() )
				xValues.push_back(interval.m_para[Interval::IP_END].value);
			// last interval
			else
			{
				xValues.push_back(24. * 3600.);
			}
			// yValues: backward definition, fill the value of the current interval at its start value
			yValues.push_back(paraIt->second.get_value(quantityUnit) );
		}
		// yValues: backward definition, fill the value of the first interval to the last spline value
		yValues.push_back(firstParaIt->second.get_value(quantityUnit) );

		// construct a linear spline
		spline.setValues(xValues,yValues);
		std::string errmsg;
		// error generating the spline
		if(!spline.makeSpline(errmsg))
				throw IBK::Exception(IBK::FormatString("Error converting interval definition to a linear spline parameter for quantity with name %1: "
									"The error message was: %2.")
									.arg(quantityName)
									.arg(errmsg),
									FUNC_ID);
	}
	else
	{
		// convert x-values
		std::vector<double> xValues(m_hourlyValues[jFirst].m_values.x().size());
		try {
			for(unsigned int i = 0; i < m_hourlyValues[jFirst].m_values.x().size(); ++i)
			{
				IBK::Parameter timeVal(m_hourlyValues[jFirst].m_name,m_hourlyValues[jFirst].m_values.x()[i],m_hourlyValues[jFirst].m_xUnit);
				xValues[i] = timeVal.get_value("s");
			}
		}
		catch(IBK::Exception &ex)
		{
			throw IBK::Exception(IBK::FormatString("Error converting hourly values of quantity %1 to linear spline. "
									"The error message was: %2")
									.arg(quantityName)
									.arg(ex.what()),
									FUNC_ID);
		}
		// constrcut the spline
		spline.setValues(xValues, m_hourlyValues[jFirst].m_values.y());
		std::string errmsg;
		// error generating the spline
		if(!spline.makeSpline(errmsg))
				throw IBK::Exception(IBK::FormatString("Error converting hourly values to a linear spline parameter for quantity with name %1: "
									"The error message was: %2.")
									.arg(quantityName)
									.arg(errmsg),
									FUNC_ID);
	}
#endif
}

void DailyCycle::checkIntervalDefinition() const {
#if 0
	const char * const FUNC_ID = "[Interval::checkIntervalDefinition]";

	if (m_intervals.empty())
		throw IBK::Exception( IBK::FormatString("Daily cycle does not have any intervals."), FUNC_ID);

	double startTime = 0.0;
	double endTime = 0.0;

	for(unsigned int i = 0; i < m_intervals.size(); ++i)
	{
		const Interval &interval = m_intervals[i];
		// fill in end and start time point
		// default: start time = endTime of the last interval
		startTime = endTime;
		// error: we have a start time > 1d: last interval was the end of
		// daily cycle, no interval definition is allowed
		if(startTime > 24.0 * 3600)
			throw IBK::Exception(IBK::FormatString("Error in Interval #%1 of for daily cycle: "
								"The interval definition exceeds the period of one day.")
								.arg(i), FUNC_ID);
		// overwrite start parameter
		if (!interval.m_para[Interval::IP_START].name.empty()) {
			startTime = interval.m_para[Interval::IP_START].value;
			// start time must equal end time of the previous interval
			if(std::fabs(startTime - endTime) > 1e-10)
				throw IBK::Exception(IBK::FormatString("Error in Interval #%1 of daily cycle: "
								"The interval definition is not close. Start != End of the last interval.")
								.arg(i), FUNC_ID);
		}
		// retreive end time
		endTime = std::min(interval.endTime(), 24.0 * 3600 + 1e-10);
		// check validity of interval definition
		try {
			interval.checkParameters(true);
		}
		catch(IBK::Exception &ex)
		{
			throw IBK::Exception(ex, IBK::FormatString("Error in Interval definition for daily cycle."), FUNC_ID);
		}
		// all ok
	}
	// dayly cycle is incomplete
	if(endTime < 24.0 * 3600)
	{
		IBK::Parameter endTimePara(NANDRAD::KeywordList::Keyword("Interval::para_t", NANDRAD::Interval::IP_END),
							endTime, IBK::Unit("s"));
		throw IBK::Exception(IBK::FormatString("Error initializing daily cycle: "
								"Definition ends at time point %1 h.")
								.arg(endTimePara.get_value("h")),
								FUNC_ID);
	}
#endif
}


bool DailyCycle::operator!=(const DailyCycle & other) const {
	if (m_interpolation != other.m_interpolation) return true;
	if (m_timePoints != other.m_timePoints) return true;
	if (m_timeUnit != other.m_timeUnit) return true;
	if (m_values != other.m_values) return true;
	return false;
}


} // namespace NANDRAD

