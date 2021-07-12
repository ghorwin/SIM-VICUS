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

#ifndef NANDRAD_IdealHeatingCoolingModelH
#define NANDRAD_IdealHeatingCoolingModelH

#include <IBK_Parameter.h>

#include "NANDRAD_Constants.h"
#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*! An ideal heating and cooling model. Basically scales a heating/cooling control signal with
	the nominal heating power per zone.
*/
class IdealHeatingCoolingModel {
public:
	/*! Model parameters. */
	enum para_t {
		P_MaxHeatingPowerPerArea,	// Keyword: MaxHeatingPowerPerArea		[W/m2]		'Maximum heating power per floor area'
		P_MaxCoolingPowerPerArea,	// Keyword: MaxCoolingPowerPerArea		[W/m2]		'Maximum cooling power per floor area'
		P_Kp,						// Keyword: Kp							[---]		'Kp-parameter'
		P_Ki,						// Keyword: Ki							[---]		'Ki-parameter'
		NUM_P
	};

	NANDRAD_READWRITE
	NANDRAD_COMPARE_WITH_ID

	/*! Checks parameters for valid values. */
	void checkParameters() const;

	/*! Comparies objects by physical parametrization (excluding ID and displayname and object list). */
	bool equal(const IdealHeatingCoolingModel & other) const;

	/*! Unique ID-number for this model. */
	unsigned int		m_id = NANDRAD::INVALID_ID;					// XML:A:required
	/*! Some display/comment name for this model (optional). */
	std::string			m_displayName;								// XML:A

	/*! Object list with zones that this model is to be apply to. */
	std::string			m_zoneObjectList;							// XML:E:required

	/*! Parameters. */
	IBK::Parameter		m_para[NUM_P];								// XML:E
};

} // namespace NANDRAD

#endif // NANDRAD_IdealHeatingCoolingModelH
