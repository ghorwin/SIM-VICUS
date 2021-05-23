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

#ifndef NANDRAD_MaterialH
#define NANDRAD_MaterialH

#include <IBK_Parameter.h>

#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*! Class containing material data needed in the construction model. */
class Material {
public:

	/*! Basic parameters. */
	enum para_t {
		/*! Dry density of the material. */
		P_Density,					// Keyword: Density				[kg/m3]	'Dry density of the material.'
		/*! Specific heat capacity of the material. */
		P_HeatCapacity,				// Keyword: HeatCapacity		[J/kgK]	'Specific heat capacity of the material.'
		/*! Thermal conductivity of the dry material. */
		P_Conductivity,				// Keyword: Conductivity		[W/mK]	'Thermal conductivity of the dry material.'
		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE
	NANDRAD_COMP(Material)
	NANDRAD_COMPARE_WITH_ID

	/*! Returns true, if all parameters are the same (id and displayname are ignored). */
	bool behavesLike(const Material & other) const;

	/*! Checks for valid parameters (value ranges). */
	void checkParameters() const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique id number. */
	unsigned int				m_id;							// XML:A:required
	/*! Display name of material. */
	std::string					m_displayName;					// XML:A
	/*! List of parameters. */
	IBK::Parameter				m_para[NUM_P];					// XML:E

};

} // namespace NANDRAD

#endif // NANDRAD_MaterialH
