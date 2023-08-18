/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

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

#include "VICUS_MaterialLayer.h"

namespace VICUS {

bool MaterialLayer::isValid(const VICUS::Database<VICUS::Material> & materials) const {
	const Material * mat = materials[m_idMaterial];
	if (mat == nullptr) {
		m_errorMsg = "Material with id '" + std::to_string(m_idMaterial) +"' does not exist.";
		return false; // error, material with this ID is not found
	}

	if (!mat->isValid(false)) {
		m_errorMsg = "Material '" + mat->m_displayName.string("de", true) + "' is not valid.";
		return false;
	}

	try {
		m_para[P_Thickness].checkedValue("Thickness", "m", "m", 0, false, std::numeric_limits<double>::max(), false,
								 "Thickness > 0 is required.");
	} catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		m_errorMsg = "Material '" + mat->m_displayName.string("de", true) + "' has following error: " + ex.what();
		return false;
	}
	return true;
}


} // namespace VICUS

