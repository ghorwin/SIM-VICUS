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

AbstractDBElement::ComparisonResult EPDDataset::equal(const AbstractDBElement *other) const {
	const EPDDataset * otherEPD = dynamic_cast<const EPDDataset*>(other);
	if (otherEPD == nullptr)
		return Different;

	//first check critical data

	//check parameters
	for(unsigned int i=0; i<NUM_P; ++i){
		if(m_para[i] != otherEPD->m_para[i])
			return Different;
	}
	if(m_uuid != otherEPD->m_uuid ||
			m_referenceUnit != otherEPD->m_referenceUnit||
			m_referenceQuantity != otherEPD->m_referenceQuantity ||
			m_subtype != otherEPD->m_subtype ||
			m_category != otherEPD->m_category ||
			m_expireDate != otherEPD->m_expireDate)
		return Different;

	//check meta data

	if(m_displayName != otherEPD->m_displayName ||
			m_color != otherEPD->m_color ||
			m_dataSource != otherEPD->m_dataSource ||
			m_manufacturer != otherEPD->m_manufacturer ||
			m_notes != otherEPD->m_notes)
		return OnlyMetaDataDiffers;

	return Equal;
}

}
