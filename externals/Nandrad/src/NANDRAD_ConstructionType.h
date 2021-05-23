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

#ifndef NANDRAD_ConstructionTypeH
#define NANDRAD_ConstructionTypeH

#include <vector>
#include <string>

#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_MaterialLayer.h"
#include "NANDRAD_Constants.h"

namespace NANDRAD {

/*! Defines a multi-layered construction (without the boundary conditions). */
class ConstructionType {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE
	NANDRAD_COMP(ConstructionType)
	NANDRAD_COMPARE_WITH_ID

	/*! Checks for valid parameters in all material layers of the construction and
		also creates quick-access pointers to materials.
	*/
	void checkParameters(const std::vector<Material> & materials);


	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique id number. */
	unsigned int				m_id;									// XML:A:required

	/*! Optional active layer index (use INVALID_ID to disable). */
	unsigned int				m_activeLayerIndex = INVALID_ID;		// XML:E
	/*! IBK-language encoded name of construction. */
	std::string					m_displayName;							// XML:A

	/*! List of material layers. */
	std::vector<MaterialLayer>	m_materialLayers;						// XML:E
};

} // namespace NANDRAD

#endif // NANDRAD_ConstructionTypeH
