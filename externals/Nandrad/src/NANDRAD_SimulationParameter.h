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

#ifndef NANDRAD_SimulationParameterH
#define NANDRAD_SimulationParameterH

#include <string>
#include <IBK_Parameter.h>
#include <IBK_IntPara.h>
#include <IBK_Flag.h>

class TiXmlElement;

#include "NANDRAD_Interval.h"
#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*!	\brief Declaration for class SimulationParameters

	Simulation parameters select all parameter default values. Inside a zone or at an interface
	these parameters can be redefined. The parameter value of the internal structure is always prefered.

	Redefinitions that are possible:

	Zone: ---
	Interface:  HeatTransferResistance, AdsorbtionCoefficient

*/
class SimulationParameter {
public:


	enum para_t {
		SP_RADIATIONLOADFRACTION,				// Keyword: RadiationLoadFraction				[%]		'Percentage of solar radiation gains attributed direcly to room 0..1.'
		SP_USERTHERMALRADIATIONFRACTION,		// Keyword: UserThermalRadiationFraction		[---]	'Percentage of heat that is emitted by long wave radiation from persons.'
		SP_EQUIPMENTTHERMALLOSSFRACTION,		// Keyword: EquipmentThermalLossFraction		[---]	'Percentage of energy from equipment load that is not available as thermal heat.'
		SP_EQUIPMENTTHERMALRADIATIONFRACTION,	// Keyword: EquipmentThermalRadiationFraction	[---]	'Percentage of heat that is emitted by long wave radiation from equipment.'
		SP_LIGHTINGVISIBLERADIATIONFRACTION,	// Keyword: LightingVisibleRadiationFraction	[---]	'Percentage of energy from lighting that is transformed into visible short wave radiation.'
		SP_LIGHTINGTHERMALRADIATIONFRACTION,	// Keyword: LightingThermalRadiationFraction	[---]	'Percentage of heat that is emitted by long wave radiation from lighting.'
		SP_DOMESTICWATERHEATGAINFRACTION,		// Keyword: DomesticWaterSensitiveHeatGainFraction	[---]	'Percentage of sensitive heat from domestic water istributed towrads the room.'
		SP_AIREXCHANGERATEN50,					// Keyword: AirExchangeRateN50					[1/h]	'Air exchange rate resulting from a pressure difference of 50 Pa between inside and outside.'
		SP_SHIELDINGCOEFFICIENT,				// Keyword: ShieldingCoefficient				[---]	'Shielding coefficient for a given location and envelope type.'
		SP_HEATINGDESIGNAMBIENTTEMPERATURE,		// Keyword: HeatingDesignAmbientTemperature		[C]		'Ambient temparture for a design day. Parameter that is needed for FMU export.'
		NUM_SP
	};
	enum intpara_t {
		SIP_YEAR,								// Keyword: StartYear									'Start year of the simulation.'
		NUM_SIP
	};

	enum flag_t {
		SF_ENABLE_MOISTURE_BALANCE,				// Keyword: EnableMoistureBalance						'Flag activating moisture balance calculation if enabled.'
		SF_ENABLE_CO2_BALANCE,					// Keyword: EnableCO2Balance							'Flag activating CO2 balance calculation if enabled.'
		SF_ENABLE_JOINT_VENTILATION,			// Keyword: EnableJointVentilation						'Flag activating ventilation through joints and openings.'
		SF_EXPORT_CLIMATE_DATA_FMU,				// Keyword: ExportClimateDataFMU						'Flag activating FMU export of climate data.'
		NUM_SF
	};


	/*! Sets default values. */
	void initDefaults();

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Reads the data from the xml element.
		Throws an IBK::Exception if a syntax error occurs.
	*/
	//void readXML(const TiXmlElement * element);

	/*! Appends the element to the parent xml element.
		Throws an IBK::Exception in case of invalid data.
	*/
	//TiXmlElement * writeXML(TiXmlElement * parent) const;

	NANDRAD_READWRITE

	/*! Compares this instance with another by value and returns true if they differ. */
	bool operator!=(const SimulationParameter & other) const;

	/*! Compares this instance with another by value and returns true if they are the same. */
	bool operator==(const SimulationParameter & other) const { return ! operator!=(other); }

	// *** PUBLIC MEMBER VARIABLES ***

	/*! List of parameters. */
	IBK::Parameter		m_para[NUM_SP];					// XML:E
	/*! Integer parameters. */
	IBK::IntPara		m_intpara[NUM_SIP];				// XML:E
	/*! List of flags. */
	IBK::Flag			m_flags[NUM_SF];				// XML:E

	/*! The time interval of simulation beginning, offset
		and duration from January 1, 0:00 of the start year. */
	Interval			m_interval;						// XML:E
};

} // namespace NANDRAD

#endif // SimulationParameterH
