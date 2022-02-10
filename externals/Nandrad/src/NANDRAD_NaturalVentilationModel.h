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

/*! Contains all data for natural ventilation models.
	\note For variant `Scheduled` there must be a schedule with name 'VentilationRateSchedule' defined.

	For variant `ScheduledWithBaseACR` there must be a schedule with name 'VentilationRateSchedule' defined, which marks
	the base ventilation rate. Also, another schedule name 'VentilationRateIncreaseSchedule' must be defined, which
	holds the _additional_ ventilation rate to be _added_ to the base ventilation rate,
	when (constant parameter) conditions apply.

	For variant `ScheduledWithBaseACRDynamicTLimit` there must be a schedule with name 'VentilationRateSchedule' defined,
	which marks the base ventilation rate. Also, another schedule name 'VentilationRateIncreaseSchedule' must be defined,
	which holds the _additional_ ventilation rate to be _added_ to the base ventilation rate,
	when (scheduled parameter) conditions apply.

*/
class NaturalVentilationModel {
public:
	/*! Different model variants. */
	enum modelType_t {
		/*! Ventilation rate is given as constant parameter. */
		MT_Constant,							// Keyword: Constant							'Constant ventilation rate (also can used as infiltration)'
		/*! Ventilation rate is provided as 'VentilationRateSchedule' schedule parameter. */
		MT_Scheduled,							// Keyword: Scheduled							'Scheduled ventilation rate'
		/*! Increased ventilation when thermal conditions apply. */
		MT_ScheduledWithBaseACR,				// Keyword: ScheduledWithBaseACR				'Scheduled basic air exchange (infiltration) with an additional increased air exchange (ventilation) if the (constant) control conditions are met. '
		/*! Increased ventilation whith scheduled minimum and maximum temperature. */
		MT_ScheduledWithBaseACRDynamicTLimit,	// Keyword: ScheduledWithBaseACRDynamicTLimit	'Scheduled basic air exchange (infiltration) with an additional increased air exchange and scheduled minimum/maximum temperature limits. '
		NUM_MT
	};

	/*! Model parameters. */
	enum para_t {
		P_VentilationRate,				// Keyword: VentilationRate					[1/h]	'Ventilation rate for Constant model'
		P_VentilationMaxAirTemperature,	// Keyword: VentilationMaxAirTemperature	[C]		'Upper limit of comfort range'
		P_VentilationMinAirTemperature,	// Keyword: VentilationMinAirTemperature	[C]		'Lower limit of comfort range'
		P_MaxWindSpeed,					// Keyword: MaxWindSpeed					[m/s]	'Maximum wind speed to allow ventilation increase'
		NUM_P
	};

	NANDRAD_READWRITE
	NANDRAD_COMPARE_WITH_ID

	/*! Checks parameters for valid values. */
	void checkParameters() const;

	/*! Comparies objects by physical parametrization (excluding ID and displayname and object list). */
	bool equal(const NaturalVentilationModel & other) const;

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
