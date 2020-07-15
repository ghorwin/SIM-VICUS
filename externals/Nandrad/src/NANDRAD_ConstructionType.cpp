#include "NANDRAD_ConstructionType.h"

namespace NANDRAD {

bool ConstructionType::operator!=(const ConstructionType & other) const {
	if (m_id != other.m_id) return true;
	if (m_displayName != other.m_displayName) return true;
	if (m_materialLayers != other.m_materialLayers) return true;
	return  false;
}

} // namespace NANDRAD
