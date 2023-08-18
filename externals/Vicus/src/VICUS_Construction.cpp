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

#include "VICUS_Construction.h"

namespace VICUS {

bool Construction::isValid(const VICUS::Database<Material> & materials) const {
	for (unsigned int i=0; i<m_materialLayers.size(); ++i) {
		if (!m_materialLayers[i].isValid(materials)) {
			// NOTE: isValid() returns false also if material does not exist, which can happen for newly constructed constructions
			const Material * mat = materials[m_materialLayers[i].m_idMaterial];
			if (mat == nullptr)
				m_errorMsg = "No material referenced in this layer, yet";
			else
				m_errorMsg = "Material '" + mat->m_displayName.string("de", true) + "' is not valid.";
			return false; // error, invalid layer thickness
		}
	}
	return true;
}


bool Construction::calculateUValue(double & UValue, const VICUS::Database<Material> & materials, double ri, double re) const {
	// simple calculation
	double R = ri + re;
	for (unsigned int i=0; i<m_materialLayers.size(); ++i) {
		const Material * mat = materials[m_materialLayers[i].m_idMaterial];
		if (mat == nullptr)
			return false; // error, material with this ID is not found
		double lambda, thickness;
		if ( (lambda = mat->m_para[Material::P_Conductivity].value) <= 0)
			return false; // error, invalid lambda
		if ( (thickness = m_materialLayers[i].m_para[MaterialLayer::P_Thickness].value) <= 0)
			return false; // error, invalid layer thickness
		R += thickness/lambda;
	}
	UValue = 1/(R+1e-10);
	return true;
}


AbstractDBElement::ComparisonResult Construction::equal(const AbstractDBElement *other) const{
	const Construction * otherConstr = dynamic_cast<const Construction*>(other);
	if (otherConstr == nullptr)
		return Different;

	if (m_materialLayers != otherConstr->m_materialLayers)
		return Different;

	if (	m_displayName != otherConstr->m_displayName ||
			m_dataSource != otherConstr->m_dataSource ||
			m_notes != otherConstr->m_notes ||
			m_usageType != otherConstr->m_usageType ||
			m_insulationKind != otherConstr->m_insulationKind ||
			m_materialKind != otherConstr->m_materialKind)
		return OnlyMetaDataDiffers;

	return Equal;
}


} // namespace VICUS
