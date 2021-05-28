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

#ifndef NANDRAD_NaturalVentilationModelH
#define NANDRAD_NaturalVentilationModelH

#include <IBK_Parameter.h>

#include "NANDRAD_Constants.h"
#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*! Contains all data for natural ventilation models. */
class NaturalVentilationModel {
public:
	/*! Different model variants. */
	enum modelType_t {
		/*! Ventilation rate is given as constant parameter. */
		MT_Constant,					// Keyword: Constant				'Constant infiltration rate'
		/*! Ventilation rate is provided as 'InfiltrationRateSchedule' schedule parameter. */
		MT_Scheduled,					// Keyword: Scheduled				'Scheduled ventilation rate'
		/*! Increased day time ventilation. */
		MT_ScheduledWithBaseACR,		// Keyword: ScheduledWithBaseACR	'Constant basic air exchange (infiltration) with an additional increased air exchange if the control conditions are met. '
		NUM_MT
	};

	/*! Model parameters. */
	enum para_t {
		P_VentilationRate,							// Keyword: VentilationRate							[1/h]		'Ventilation rate'
		P_MaximumRoomAirTemperatureACRLimit,		// Keyword: MaximumRoomAirTemperatureACRLimit		[C]			'Condition for ventilation increase'
		P_MinimumRoomAirTemperatureACRLimit,		// Keyword: MinimumRoomAirTemperatureACRLimit		[C]			'Condition for ventilation increase'
		P_MaximumEnviromentAirTemperatureACRLimit,	// Keyword: MaximumEnviromentAirTemperatureACRLimit	[C]			'Condition for ventilation increase'
		P_MinimumEnviromentAirTemperatureACRLimit,	// Keyword: MinimumEnviromentAirTemperatureACRLimit	[C]			'Condition for ventilation increase'
		P_DeltaTemperatureACRLimit,					// Keyword: DeltaTemperatureACRLimit				[K]			'Condition for ventilation increase'
		P_WindSpeedACRLimit,						// Keyword: WindSpeedACRLimit						[m/s]		'Condition for ventilation increase'
		NUM_P
	};

	NANDRAD_READWRITE

	/*! Checks parameters for valid values. */
	void checkParameters() const;

	/*! Unique ID-number for this ventilation rate model. */
	unsigned int						m_id = NANDRAD::INVALID_ID;		// XML:A:required
	/*! Some display/comment name for this model (optional). */
	std::string							m_displayName;					// XML:A

	/*! Model type. */
	modelType_t							m_modelType = NUM_MT;			// XML:A:required

	/*! Object list with zones that this model is to be apply to. */
	std::string							m_zoneObjectList;				// XML:E:required

	/*! Model parameters. */
	IBK::Parameter						m_para[NUM_P];					// XML:E
};

} // namespace NANDRAD

#endif // NANDRAD_NaturalVentilationModelH
