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

#include "VICUS_Material.h"

#include <VICUS_KeywordList.h>

namespace VICUS {

AbstractDBElement::~AbstractDBElement() {
}


Material::Material( unsigned int id, const IBK::MultiLanguageString &name,
					double conductivity, double density, double specHeatCapa) :
	AbstractDBElement(id, name)
{
	VICUS::KeywordList::setParameter(m_para, "Material::para_t", P_Density, density);
	VICUS::KeywordList::setParameter(m_para, "Material::para_t", P_Conductivity, conductivity);
	VICUS::KeywordList::setParameter(m_para, "Material::para_t", P_HeatCapacity, specHeatCapa);
}


NANDRAD::Material Material::toNandrad() const {
	NANDRAD::Material mat;
	mat.m_id = m_id;
	mat.m_para[NANDRAD::Material::P_Density] = m_para[NANDRAD::Material::P_Density];
	mat.m_para[NANDRAD::Material::P_Conductivity] = m_para[NANDRAD::Material::P_Conductivity];
	mat.m_para[NANDRAD::Material::P_HeatCapacity] = m_para[NANDRAD::Material::P_HeatCapacity];
	return mat;
}


bool Material::isValid(bool hygrothermalCalculation) const {

	NANDRAD::Material mat = toNandrad();
	try {
		mat.checkParameters();
	} catch (IBK::Exception &ex) {
		m_errorMsg = ex.what();
		return false;
	}

	if (hygrothermalCalculation) {
		// TODO : additional checks for hygrothermal parameters
	}

	return true;
}


AbstractDBElement::ComparisonResult Material::equal(const AbstractDBElement * other) const {
	const Material * otherMaterial = dynamic_cast<const Material*>(other);
	if (otherMaterial == nullptr)
		return Different;

	for (unsigned int i=0; i<NUM_P; ++i) {
		if (m_para[i] != otherMaterial->m_para[i])
			return Different;
	}

	if (m_epdCategorySet != otherMaterial->m_epdCategorySet)
		return Different;

	// check meta data

	if (m_displayName != otherMaterial->m_displayName ||
		m_category != otherMaterial->m_category ||
		m_notes != otherMaterial->m_notes ||
		m_dataSource != otherMaterial->m_dataSource ||
		m_manufacturer != otherMaterial->m_manufacturer)
		return OnlyMetaDataDiffers;

	// Note: color differences do not count as difference

	return Equal;
}


} // namespace VICUS
