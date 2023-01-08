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

#ifndef NANDRAD_InternalMoistureLoadsModelH
#define NANDRAD_InternalMoistureLoadsModelH

#include <IBK_Parameter.h>

#include "NANDRAD_Constants.h"
#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_KeywordList.h"
#include "NANDRAD_SimulationParameter.h"

namespace NANDRAD {

/*! Contains schedules or constant parameters for internal loads (moisture production rate, related to zone area). */
class InternalMoistureLoadsModel {
public:
	/*! Different model variants. */
	enum modelType_t {
		/*! Internal loads are given with constant parameters. */
		MT_Constant,			// Keyword: Constant	'Constant internal loads'
		/*! Internal loads are provided via schedule parameters. */
		MT_Scheduled,			// Keyword: Scheduled	'Scheduled internal loads'
		NUM_MT
	};

	/*! Model parameters. */
	enum para_t {
		P_MoistureLoadPerArea,	// Keyword: MoistureLoadPerArea	[kg/m2s]	'Moisture load per zone floor area.'
		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE
	NANDRAD_COMPARE_WITH_ID

	/*! Checks for valid parameters (value ranges). */
	void checkParameters(const NANDRAD::SimulationParameter& simPara) const;

	/*! Comparies objects by physical parametrization (excluding ID and displayname and object list). */
	bool equal(const InternalMoistureLoadsModel & other) const;

	// *** PUBLIC MEMBER VARIABLES ***

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

#endif // NANDRAD_InternalMoistureLoadsModelH
