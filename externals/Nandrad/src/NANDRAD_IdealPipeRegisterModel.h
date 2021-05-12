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

#ifndef NANDRAD_IdealPipeRegisterModelH
#define NANDRAD_IdealPipeRegisterModelH

#include <IBK_Parameter.h>

#include "NANDRAD_Constants.h"
#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

class Zone;

/*! An ideal heating and cooling model. Basically scales a heating/cooling control signal with
	the nominal heating power per zone.
*/
class IdealPipeRegisterModel {
public:
	/*! Different model variants for supply tempertaure. */
	enum modelType_t {
		/*! Supply temperature is given as constant parameter. */
		MT_Constant,					// Keyword: Constant				'Constant infiltration rate'
		/*! Supply temperature is provided as 'SupplyTemperatureSchedule' schedule parameter. */
		MT_Scheduled,					// Keyword: Scheduled				'Scheduled ventilation rate'
		NUM_MT
	};

	/*! Model parameters. */
	enum para_t {
		P_SupplyTemperature,		// Keyword: SupplyTemperature			[C]			'Medium supply temperature'
		P_MaxMassFlow,				// Keyword: MaxMassFlow					[kg/s]		'Maximum mass flow through the pipe'
		NUM_P
	};

	NANDRAD_READWRITE

	/*! Checks parameters for valid values. */
	void checkParameters(const std::vector<NANDRAD::Zone> &zones) const;

	/*! Unique ID-number for this model. */
	unsigned int		m_id = NANDRAD::INVALID_ID;					// XML:A:required
	/*! Some display/comment name for this model (optional). */
	std::string			m_displayName;								// XML:A

	/*! Model type. */
	modelType_t			m_modelType = NUM_MT;						// XML:A:required

	/*! Object list with zones that this model is to be apply to. */
	std::string			m_constructionObjectList;					// XML:E:required

	/*! Id of zone whose thermostat is used for control (one zone thermostat may be responsible
		for the control of different heating systems, but only one id is allowed per model). */
	unsigned int		m_thermostatZoneID;							// XML:E:required

	/*! Parameters. */
	IBK::Parameter		m_para[NUM_P];								// XML:E
};

} // namespace NANDRAD

#endif // NANDRAD_IdealPipeRegisterModelH
