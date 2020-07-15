#include "NANDRAD_Material.h"

namespace NANDRAD {

bool Material::operator!=(const Material & other) const
{

	for (size_t i=0; i<NUM_MP; ++i) {
		if(m_para[i].get_value() != other.m_para[i].get_value())
			return true;
	}

	return false;
}

}
