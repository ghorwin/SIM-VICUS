#include "NANDRAD_Material.h"

namespace NANDRAD {

bool Material::operator!=(const Material & other) const {
	if (m_id != other.m_id)
		return true;
	if (m_displayName != other.m_displayName)
		return true;
	for (size_t i=0; i<NUM_MP; ++i) {
		if (m_para[i] != other.m_para[i])
			return true;
	}
	return false;
}


bool Material::behavesLike(const Material & other) const {
	for (size_t i=0; i<NUM_MP; ++i) {
		if (m_para[i] != other.m_para[i])
			return false;
	}
	return true;
}

} // namespace NANDRAD
