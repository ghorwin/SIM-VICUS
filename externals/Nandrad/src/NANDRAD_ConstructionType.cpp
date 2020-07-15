#include "NANDRAD_ConstructionType.h"


namespace NANDRAD {


bool ConstructionType::operator!=(const ConstructionType & other) const
{
	if(m_materialLayers.size() != other.m_materialLayers.size())
		return true;

	for (size_t i=0; i<m_materialLayers.size(); ++i) {
		if( (m_materialLayers[i].m_thickness != other.m_materialLayers[i].m_thickness) ||
				m_materialLayers[i].m_matId != other.m_materialLayers[i].m_matId)
			return  true;
	}
	return  false;
}


}
