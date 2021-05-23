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

#ifndef NANDRAD_SimulationParameterH
#define NANDRAD_SimulationParameterH

#include <string>
#include <IBK_Parameter.h>
#include <IBK_IntPara.h>
#include <IBK_Flag.h>

#include "NANDRAD_Interval.h"
#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_SolarLoadsDistributionModel.h"

namespace NANDRAD {

class LinearSplineParameter;

/*!	Simulation parameters define global model settings. */
class SimulationParameter {
public:

	/*! Parameter values. */
	enum para_t {
		P_InitialTemperature,						// Keyword: InitialTemperature					[C]		'Global initial temperature [C].'
		P_InitialRelativeHumidity,					// Keyword: InitialRelativeHumidity				[%]		'Global initial relative humidity [%].'
		P_DomesticWaterSensitiveHeatGainFraction,	// Keyword: DomesticWaterSensitiveHeatGainFraction	[---]	'Percentage of sensitive heat from domestic water istributed towrads the room.'
		P_AirExchangeRateN50,						// Keyword: AirExchangeRateN50					[1/h]	'Air exchange rate resulting from a pressure difference of 50 Pa between inside and outside.'
		P_ShieldingCoefficient,						// Keyword: ShieldingCoefficient				[---]	'Shielding coefficient for a given location and envelope type.'
		P_HeatingDesignAmbientTemperature,			// Keyword: HeatingDesignAmbientTemperature		[C]		'Ambient temperature for a design day. Parameter that is needed for FMU export.'
		NUM_P
	};

	/*! Integer parameters. */
	enum intPara_t {
		IP_StartYear,								// Keyword: StartYear									'Start year of the simulation.'
		NUM_IP
	};

	/*! Global model flags. */
	enum flag_t {
		F_EnableMoistureBalance,					// Keyword: EnableMoistureBalance						'Flag activating moisture balance calculation if enabled.'
		F_EnableCO2Balance,							// Keyword: EnableCO2Balance							'Flag activating CO2 balance calculation if enabled.'
		F_EnableJointVentilation,					// Keyword: EnableJointVentilation						'Flag activating ventilation through joints and openings.'
		F_ExportClimateDataFMU,						// Keyword: ExportClimateDataFMU						'Flag activating FMU export of climate data.'
		NUM_F
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Init default values (called before readXML()).
		\note These values will be overwritten in readXML() when the respective property is set
			  in the project file.
	*/
	void initDefaults();

	NANDRAD_READWRITE
	NANDRAD_COMP(SimulationParameter)

	/*! To be called after readXML() and mainly used to check whether user-provided parameters are in the valid ranges. */
	void checkParameters() const;

	/*! Takes the linear spline parameter and evalutes the data using potential time shift defined due to
		simulation start and also considers continuous/cyclic usage of the spline.
		\param t Time point in simulation time (i.e. simulation always starts with 0)
		\param spl The spline data.
	*/
	double evaluateTimeSeries(double t, const NANDRAD::LinearSplineParameter & spl) const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! List of parameters. */
	IBK::Parameter		m_para[NUM_P];							// XML:E
	/*! Integer parameters. */
	IBK::IntPara		m_intPara[NUM_IP];						// XML:E
	/*! List of flags. */
	IBK::Flag			m_flags[NUM_F];							// XML:E

	/*! The time interval of simulation beginning, offset
		and duration from January 1, 0:00 of the start year. */
	Interval			m_interval;								// XML:E

	SolarLoadsDistributionModel	m_solarLoadsDistributionModel;	// XML:E
};

} // namespace NANDRAD

#endif // NANDRAD_SimulationParameterH
