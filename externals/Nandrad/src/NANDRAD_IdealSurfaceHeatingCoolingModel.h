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

#ifndef NANDRAD_IdealSurfaceHeatingCoolingModelH
#define NANDRAD_IdealSurfaceHeatingCoolingModelH

#include <IBK_Parameter.h>

#include "NANDRAD_Constants.h"
#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Zone.h"

namespace NANDRAD {

/*! An ideal heating and cooling model. Basically scales a heating/cooling control signal with
	the nominal heating power per area of heated/cooled construction.
*/
class IdealSurfaceHeatingCoolingModel {
public:
	/*! Model parameters. */
	enum para_t {
		P_MaxHeatingPowerPerArea,	// Keyword: MaxHeatingPowerPerArea		[W/m2]		'Maximum heating power per surface area'
		P_MaxCoolingPowerPerArea,	// Keyword: MaxCoolingPowerPerArea		[W/m2]		'Maximum cooling power per surface area'
		NUM_P
	};

	NANDRAD_READWRITE
	NANDRAD_COMPARE_WITH_ID

	/*! Checks parameters for valid values. */
	void checkParameters(const std::vector<NANDRAD::Zone> & zones);

	/*! Comparies objects by physical parametrization (excluding ID and displayname and object list). */
	bool equal(const IdealSurfaceHeatingCoolingModel & other) const;

	/*! Unique ID-number for this model. */
	unsigned int		m_id = NANDRAD::INVALID_ID;					// XML:A:required
	/*! Some display/comment name for this model (optional). */
	std::string			m_displayName;								// XML:A

	/*! Object list with zones that this model is to be apply to. */
	std::string			m_constructionObjectList;					// XML:E:required

	/*! Id of zone whose thermostat is used for control (one zone thermostat may be responsible
		for the control of different heating systems, but only one id is allowed per model). */
	unsigned int		m_thermostatZoneId;							// XML:E:required

	/*! Parameters. */
	IBK::Parameter		m_para[NUM_P];								// XML:E
};

} // namespace NANDRAD

#endif // NANDRAD_IdealSurfaceHeatingCoolingModelH
