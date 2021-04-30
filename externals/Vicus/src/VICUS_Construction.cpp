#include "VICUS_Construction.h"

namespace VICUS {

bool Construction::isValid(const VICUS::Database<Material> & materials) const {
	for (unsigned int i=0; i<m_materialLayers.size(); ++i) {
		const Material * mat = materials[m_materialLayers[i].m_matId];
		if (mat == nullptr)
			return false; // error, material with this ID is not found
		if ( mat->m_para[Material::P_Conductivity].value <= 0)
			return false; // error, invalid lambda
		if ( m_materialLayers[i].m_thickness.value <= 0)
			return false; // error, invalid layer thickness
	}
	return true;
}


bool Construction::calculateUValue(double & UValue, const VICUS::Database<Material> & materials, double ri, double re) const {
	// simple calculation
	double R = ri + re;
	for (unsigned int i=0; i<m_materialLayers.size(); ++i) {
		const Material * mat = materials[m_materialLayers[i].m_matId];
		if (mat == nullptr)
			return false; // error, material with this ID is not found
		double lambda, thickness;
		if ( (lambda = mat->m_para[Material::P_Conductivity].value) <= 0)
			return false; // error, invalid lambda
		if ( (thickness = m_materialLayers[i].m_thickness.value) <= 0)
			return false; // error, invalid layer thickness
		R += thickness/lambda;
	}
	UValue = 1/R;
	return true;
}

AbstractDBElement::ComparisonResult Construction::equal(const AbstractDBElement *other) const{
	const Construction * otherConstr = dynamic_cast<const Construction*>(other);
	if (otherConstr == nullptr)
		return Different;

	//first check critical data

	if(m_materialLayers != otherConstr->m_materialLayers)
		return Different;

	//check meta data

	if(m_displayName != otherConstr->m_displayName ||
			m_color != otherConstr->m_color ||
			m_dataSource != otherConstr->m_dataSource ||
			m_notes != otherConstr->m_notes ||
			m_usageType != otherConstr->m_usageType ||
			m_insulationKind != otherConstr->m_insulationKind ||
			m_materialKind != otherConstr->m_materialKind)
		return OnlyMetaDataDiffers;

	return Equal;
}


} // namespace VICUS
