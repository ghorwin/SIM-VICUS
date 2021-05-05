#include "VICUS_Infiltration.h"

#include "VICUS_KeywordList.h"

namespace VICUS {



bool Infiltration::isValid() const
{
	if(m_id == INVALID_ID)
		return false;

	if(m_airChangeType == NUM_AC)
		return false;

	try {
		m_para[P_AirChangeRate].checkedValue(VICUS::KeywordList::Keyword("Infiltration::para_t", P_AirChangeRate),
							 "1/h", "1/h", 0, true, 100, true, nullptr);

	}  catch (...) {
		return false;
	}

	if(m_airChangeType == AC_n50){
		try {
			m_para[P_ShieldingCoefficient].checkedValue(VICUS::KeywordList::Keyword("Infiltration::para_t", P_ShieldingCoefficient),
														"-", "-", 0, true, 10, true, nullptr);

		}  catch (...) {
			return false;
		}
	}
	return true;
}

AbstractDBElement::ComparisonResult Infiltration::equal(const AbstractDBElement *other) const{
	const Infiltration * otherInf = dynamic_cast<const Infiltration*>(other);
	if (otherInf == nullptr)
		return Different;

	//first check critical data

	//check parameters
	for(unsigned int i=0; i<NUM_P; ++i){
		if(m_para[i] != otherInf->m_para[i])
			return Different;
	}
	if(m_airChangeType != otherInf->m_airChangeType)
		return Different;

	//check meta data

	if(m_displayName != otherInf->m_displayName ||
			m_color != otherInf->m_color ||
			m_dataSource != otherInf->m_dataSource ||
			m_notes != otherInf->m_notes)
		return OnlyMetaDataDiffers;

	return Equal;
}

}
