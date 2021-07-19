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

#include "NANDRAD_ConstructionType.h"

#include <algorithm>

#include <IBK_Exception.h>

#include "NANDRAD_Material.h"

namespace NANDRAD {

bool ConstructionType::operator!=(const ConstructionType & other) const {
	if (m_id != other.m_id) return true;
	if (m_displayName != other.m_displayName) return true;
	if (m_materialLayers != other.m_materialLayers) return true;
	return  false;
}


void ConstructionType::checkParameters(const std::vector<Material> & materials) {
	FUNCID(ConstructionType::checkParameters);

	if (m_materialLayers.empty())
		throw IBK::Exception(IBK::FormatString("Missing material layers."), FUNC_ID);

	// process all material layers
	for (MaterialLayer & ml : m_materialLayers) {
		// valid layer thickness
		if (ml.m_thickness <= 0)
			throw IBK::Exception("Invalid thickness in material layer.", FUNC_ID);

		// now look for referenced material
		std::vector<Material>::const_iterator it = std::find(materials.begin(), materials.end(), ml.m_matId);
		if (it == materials.end())
			throw IBK::Exception( IBK::FormatString("Invalid material ID %1.").arg(ml.m_matId), FUNC_ID);
		ml.m_material = &(*it); // store pointer
	}

	// check validity of active layer
	if (m_activeLayerIndex != NANDRAD::INVALID_ID && m_activeLayerIndex >= m_materialLayers.size())
		throw IBK::Exception( IBK::FormatString("Active layer index %1 exceeds number of material layers (%2). Mind: layer index is zero-based!")
							  .arg(m_activeLayerIndex).arg(m_materialLayers.size()), FUNC_ID);
}

} // namespace NANDRAD
