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


} // namespace VICUS
