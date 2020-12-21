#include "VICUS_Material.h"

#include <VICUS_KeywordList.h>

namespace VICUS {

Material::Material( unsigned int id, const IBK::MultiLanguageString &name,
					double conductivity, double density, double specHeatCapa) :
	m_id(id),
	m_displayName(name)
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
	} catch (...) {
		return false;
	}

	if (hygrothermalCalculation) {
		// TODO : additional checks for hygrothermal parameters
	}

	return true;
}


}
