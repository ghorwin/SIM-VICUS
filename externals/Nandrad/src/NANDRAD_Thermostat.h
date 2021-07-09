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

#ifndef NANDRAD_ThermostatH
#define NANDRAD_ThermostatH

#include <IBK_Parameter.h>

#include "NANDRAD_Constants.h"
#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*! Contains data for a thermostat model, that compares the air/operative tempeature with
	a setpoint and generates either an analog or digital control signal. The latter always uses
	hysteresis.
*/
class Thermostat {
public:
	/*! Different model variants regarding source of setpoint parametrization. */
	enum modelType_t {
		/*! Setpoints are constant parameters. */
		MT_Constant,					// Keyword: Constant				'Constant set points'
		/*! Setpoints are provided as 'HeatingSetPointSchedule' and 'CoolingSetPointSchedule' schedule parameter. */
		MT_Scheduled,					// Keyword: Scheduled				'Scheduled set points'
		NUM_MT
	};

	/*! Model parameters. */
	enum para_t {
		P_HeatingSetpoint,				// Keyword: HeatingSetpoint				[C]			'Heating set point'
		P_CoolingSetpoint,				// Keyword: CoolingSetpoint				[C]			'Cooling set point'
		P_TemperatureTolerance,			// Keyword: TemperatureTolerance		[K]			'Control tolerance for temperatures'
		P_TemperatureBand,				// Keyword: TemperatureBand				[K]			'Offset of lower and upper hysteresis band from set points'
		NUM_P
	};

	/*! Control temperature to use. */
	enum TemperatureType {
		TT_AirTemperature,				// Keyword: AirTemperature				'Air temperature'
		TT_OperativeTemperature,		// Keyword: OperativeTemperature		'Operative temperature'
		NUM_TT
	};

	enum ControllerType {
		/*! Signal is the difference between setpoint temperature and sensor temperature, normalized by
			the given temperature tolerance. A signal value of 1 means that the temperature tolerance has been
			reached, a value larger than 1 means the temperature difference is even larger (signal value is not clipped).
		*/
		CT_Analog,						// Keyword: Analog						'Analog'
		/*! Signal is either 0 or 1, and corresponds with the digital hysteretic control logic. 1 is returned
			if heating/cooling is required.
		*/
		CT_Digital,						// Keyword: Digital						'Digital'
		NUM_CT
	};

	NANDRAD_READWRITE
	NANDRAD_COMPARE_WITH_ID

	/*! Checks parameters for valid values. */
	void checkParameters() const;

	/*! Comparies objects by physical parametrization (excluding ID and displayname and object list). */
	bool equal(const Thermostat & other) const;

	/*! Unique ID-number for this ventilation rate model. */
	unsigned int						m_id = NANDRAD::INVALID_ID;						// XML:A:required
	/*! Some display/comment name for this model (optional). */
	std::string							m_displayName;									// XML:A

	/*! Model type. */
	modelType_t							m_modelType = NUM_MT;							// XML:A:required

	/*! Object list with zones that this model is to be apply to. */
	std::string							m_zoneObjectList;								// XML:E:required

	/*! Model parameters. */
	IBK::Parameter						m_para[NUM_P];									// XML:E

	/*! Optional ID of reference zone to use for sensor and setpoint input. */
	unsigned int						m_referenceZoneId = NANDRAD::INVALID_ID;		// XML:E

	/*! Temperature sensor type. */
	TemperatureType						m_temperatureType = NUM_TT;						// XML:E

	/*! The controller to use for the thermostat. */
	ControllerType						m_controllerType = NUM_CT;						// XML:E
};

} // namespace NANDRAD

#endif // NANDRAD_ThermostatH
