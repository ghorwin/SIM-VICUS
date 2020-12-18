#include "VICUS_EPDDataset.h"

namespace VICUS {

bool EPDDataset::behavesLike(const EPDDataset & other) const
{

	if(m_referenceUnit != other.m_referenceUnit ||
			m_referenceQuantity != other.m_referenceQuantity)
		return false;

	if(m_category != other.m_category)
		return false;

	for (unsigned int i=0; i<NUM_P; ++i) {
		para_t t = static_cast<para_t>(i);
		if(m_para[t].empty() && other.m_para[t].empty())
			continue;
		if(m_para[t] != other.m_para[t])
			return false;
	}

	return true;

}

}
