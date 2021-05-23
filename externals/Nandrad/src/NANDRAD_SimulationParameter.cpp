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

#include "NANDRAD_SimulationParameter.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_KeywordList.h"
#include "NANDRAD_LinearSplineParameter.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <IBK_Time.h>

namespace NANDRAD {


void SimulationParameter::initDefaults() {
	KeywordList::setParameter(m_para, "SimulationParameter::para_t", P_InitialTemperature, 20);
	KeywordList::setParameter(m_para, "SimulationParameter::para_t", P_InitialRelativeHumidity, 50);
	KeywordList::setParameter(m_para, "SimulationParameter::para_t", P_DomesticWaterSensitiveHeatGainFraction, 0);

	m_intPara[IP_StartYear].set( KeywordList::Keyword("SimulationParameter::intPara_t", IP_StartYear), 2001);

	// setting flags to false is normally not necessary, since the function Flag::isEnabled() returns false for undefined flags anyway

	m_interval.m_para[NANDRAD::Interval::P_Start]		= IBK::Parameter("Start", 0, "d");
	m_interval.m_para[NANDRAD::Interval::P_End]			= IBK::Parameter("End", 365, "d");

	m_solarLoadsDistributionModel.initDefaults();
}


void SimulationParameter::checkParameters() const {
	FUNCID(SimulationParameter::checkParameters);

	int startYear = m_intPara[IP_StartYear].value;

	double duration = m_interval.m_para[NANDRAD::Interval::P_End].value - m_interval.m_para[NANDRAD::Interval::P_Start].value;
	if (duration <= 0)
		throw IBK::Exception(IBK::FormatString("End time point %1 preceedes start time point %2 (must be later than start time!)")
							 .arg(IBK::Time(startYear, m_interval.m_para[NANDRAD::Interval::P_End].value).toDateTimeFormat())
							 .arg(IBK::Time(startYear, m_interval.m_para[NANDRAD::Interval::P_Start].value).toDateTimeFormat()), FUNC_ID);

	if (m_para[P_InitialTemperature].value < 123.15)
		throw IBK::Exception(IBK::FormatString("Invalid initial temperature %1 in SimulationParameters.")
							 .arg(m_para[P_InitialTemperature].get_value("C")), FUNC_ID);


	try {
		m_solarLoadsDistributionModel.checkParameters();
	} catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Invalid/missing parameters in SolarDistributionLoadsModel.", FUNC_ID);
	}
	/// \todo Implementation of other value range checks
}


double SimulationParameter::evaluateTimeSeries(double t, const LinearSplineParameter & spl) const {
	// apply start time shift

	t += m_interval.m_para[Interval::P_Start].value;

	// if spline defines cyclic usage, limit t to full years
	if (spl.m_wrapMethod == NANDRAD::LinearSplineParameter::C_CYCLIC) {
		while (t > IBK::SECONDS_PER_YEAR)
			t -= IBK::SECONDS_PER_YEAR;
	}

	// now evaluate spline
	if (spl.m_interpolationMethod == NANDRAD::LinearSplineParameter::I_CONSTANT)
		return spl.m_values.nonInterpolatedValue(t);
	else
		return spl.m_values.value(t);
}


bool SimulationParameter::operator!=(const SimulationParameter & other) const {
	if (m_interval != other.m_interval) return true;

	for (unsigned int i=0; i<NUM_F; ++i)
		if (m_flags[i] != other.m_flags[i]) return true;
	for (unsigned int i=0; i<NUM_P; ++i)
		if (m_para[i] != other.m_para[i]) return true;
	for (unsigned int i=0; i<NUM_IP; ++i)
		if (m_intPara[i] != other.m_intPara[i]) return true;

	return false; // this and other hold the same data
}


} // namespace NANDRAD

