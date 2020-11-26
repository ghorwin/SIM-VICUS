/*	The NANDRAD data model library.

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

#include "NANDRAD_SimulationParameter.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_KeywordList.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <IBK_Time.h>

namespace NANDRAD {


void SimulationParameter::initDefaults() {

	m_para[P_InitialTemperature].set( KeywordList::Keyword("SimulationParameter::para_t", P_InitialTemperature),	 20, IBK::Unit("C"));
	m_para[P_InitialRelativeHumidity].set( KeywordList::Keyword("SimulationParameter::para_t", P_InitialRelativeHumidity),	 50, IBK::Unit("%"));

	m_para[P_UserThermalRadiationFraction].set( KeywordList::Keyword("SimulationParameter::para_t", P_UserThermalRadiationFraction),	 0.3, IBK::Unit("---"));
	m_para[P_EquipmentThermalLossFraction].set( KeywordList::Keyword("SimulationParameter::para_t", P_EquipmentThermalLossFraction),	 0.1, IBK::Unit("---"));
	m_para[P_EquipmentThermalRadiationFraction].set( KeywordList::Keyword("SimulationParameter::para_t", P_EquipmentThermalRadiationFraction),	 0.3, IBK::Unit("---"));
	m_para[P_LightingVisibleRadiationFraction].set( KeywordList::Keyword("SimulationParameter::para_t", P_LightingVisibleRadiationFraction),	 0.18, IBK::Unit("---"));
	m_para[P_LightingThermalRadiationFraction].set( KeywordList::Keyword("SimulationParameter::para_t", P_LightingThermalRadiationFraction),	 0.72, IBK::Unit("---"));
	m_para[P_DomesticWaterSensitiveHeatGainFraction].set(KeywordList::Keyword("SimulationParameter::para_t", P_DomesticWaterSensitiveHeatGainFraction), 0.0, IBK::Unit("---"));

	m_intPara[IP_StartYear].set( KeywordList::Keyword("SimulationParameter::intPara_t", IP_StartYear), 2001);

	// setting flags to false is normally not necessary, since the function Flag::isEnabled() returns false for undefined flags anyway

	m_interval.m_para[NANDRAD::Interval::P_Start]		= IBK::Parameter("Start", 0, "d");
	m_interval.m_para[NANDRAD::Interval::P_End]		= IBK::Parameter("End", 365, "d");

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

