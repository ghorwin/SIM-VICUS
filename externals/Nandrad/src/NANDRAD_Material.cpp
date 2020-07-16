#include "NANDRAD_Material.h"

namespace NANDRAD {

bool Material::operator!=(const Material & other) const {
	for (size_t i=0; i<NUM_MP; ++i) {
		if (m_para[i] != other.m_para[i])
			return true;
	}
	return false;
}

} // namespace NANDRAD
