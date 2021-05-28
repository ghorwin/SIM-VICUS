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

#include "NANDRAD_Material.h"
#include "NANDRAD_KeywordList.h"

namespace NANDRAD {

bool Material::operator!=(const Material & other) const {
	if (m_id != other.m_id)
		return true;
	if (m_displayName != other.m_displayName)
		return true;
	for (size_t i=0; i<NUM_P; ++i) {
		if (m_para[i] != other.m_para[i])
			return true;
	}
	return false;
}


bool Material::behavesLike(const Material & other) const {
	for (size_t i=0; i<NUM_P; ++i) {
		if (m_para[i] != other.m_para[i])
			return false;
	}
	return true;
}


void Material::checkParameters() const {
	// check for mandatory and required parameters
	// check for meaningful value ranges
	m_para[P_Density].checkedValue("Density", "kg/m3", "kg/m3", 0.01, false, std::numeric_limits<double>::max(), true,
								   "Density must be > 0.01 kg/m3.");
	m_para[P_HeatCapacity].checkedValue("HeatCapacity", "J/kgK", "J/kgK", 100, true, std::numeric_limits<double>::max(), true,
								   "Heat capacity must be > 100 J/kgK.");
	m_para[P_Conductivity].checkedValue("Conductivity", "W/mK", "W/mK", 1e-5, true, std::numeric_limits<double>::max(), true,
								   "Thermal conductivity must be > 1e-5 W/mK.");
}

} // namespace NANDRAD
