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


void Material::checkParameters() {
	FUNCID(Material::checkParameters);

	// check for mandatory and required parameters

	for (unsigned int i=0; i<=P_Conductivity; ++i) {
		if (m_para[i].name.empty())
			throw IBK::Exception( IBK::FormatString("Parameter '%1' missing!")
								  .arg(NANDRAD::KeywordList::Keyword("Material::para_t", (int)i)), FUNC_ID);
	}
	// check for meaningful value ranges
	m_para[P_Density].checkedValue("kg/m3", "kg/m3", 0.01, false, std::numeric_limits<double>::max(), true,
								   "Density must be > 0.01 kg/m3.");
	m_para[P_HeatCapacity].checkedValue("J/kgK", "J/kgK", 100, true, std::numeric_limits<double>::max(), true,
								   "Heat capacity must be > 100 J/kgK.");
	m_para[P_Conductivity].checkedValue("W/mK", "W/mK", 1e-5, true, std::numeric_limits<double>::max(), true,
								   "Thermal conductivity must be > 1e-5 W/mK.");
}

} // namespace NANDRAD
