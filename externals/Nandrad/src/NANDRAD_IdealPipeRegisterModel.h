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

#ifndef NANDRAD_IdealPipeRegisterModelH
#define NANDRAD_IdealPipeRegisterModelH

#include <IBK_IntPara.h>
#include <IBK_Parameter.h>

#include "NANDRAD_Constants.h"
#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_LinearSplineParameter.h"
#include "NANDRAD_HydraulicFluid.h"

namespace NANDRAD {

class Zone;

/*! An idealised pipe-register model inside a heated layer of a construction.
	Supply temperature is fixed, mass flux is controlled based on heating/cooling requirements in
	associated thermostat.
*/
class IdealPipeRegisterModel {
public:
	/*! Different model variants for supply temperature. */
	enum modelType_t {
		/*! Supply temperature is given as constant parameter. */
		MT_Constant,					// Keyword: Constant				'Constant supply temperature'
		/*! Supply temperature is provided as 'SupplyTemperatureSchedule' schedule parameter. */
		MT_Scheduled,					// Keyword: Scheduled				'Scheduled supply temperature'
		NUM_MT
	};

	/*! Model parameters. */
	enum para_t {
		P_SupplyTemperature,		// Keyword: SupplyTemperature			[C]			'Medium supply temperature'
		P_MaxMassFlux,				// Keyword: MaxMassFlux					[kg/s]		'Maximum mass flux through the pipe'
		P_PipeLength,				// Keyword: PipeLength					[m]			'Pipe length'
		P_PipeInnerDiameter,		// Keyword: PipeInnerDiameter			[mm]		'Inner diameter of pipe'
		P_UValuePipeWall,			// Keyword: UValuePipeWall				[W/mK]		'Length-specific U-Value of pipe wall incl. insulation'
		NUM_P
	};

	/*! Whole number parameters. */
	enum intPara_t {
		IP_NumberParallelPipes,		// Keyword: NumberParallelPipes						'Number of parallel pipes'
		NUM_IP
	};

	NANDRAD_READWRITE
	NANDRAD_COMPARE_WITH_ID

	/*! Checks parameters for valid values. */
	void checkParameters(const std::vector<NANDRAD::Zone> &zones);

	/*! Comparies objects by physical parametrization (excluding ID and displayname and object list). */
	bool equal(const IdealPipeRegisterModel & other) const;

	/*! Unique ID-number for this model. */
	unsigned int		m_id = NANDRAD::INVALID_ID;					// XML:A:required
	/*! Some display/comment name for this model (optional). */
	std::string			m_displayName;								// XML:A

	/*! Model type. */
	modelType_t			m_modelType = NUM_MT;						// XML:A:required

	/*! Fluid properties. */
	HydraulicFluid		m_fluid;									// XML:E:required

	/*! Object list with zones that this model is to be apply to. */
	std::string			m_constructionObjectList;					// XML:E:required

	/*! Id of zone whose thermostat is used for control. */
	unsigned int		m_thermostatZoneId;							// XML:E:required

	/*! Parameters. */
	IBK::Parameter		m_para[NUM_P];								// XML:E
	/*! Integaer parameters. */
	IBK::IntPara		m_intPara[NUM_IP];							// XML:E
};

} // namespace NANDRAD

#endif // NANDRAD_IdealPipeRegisterModelH
