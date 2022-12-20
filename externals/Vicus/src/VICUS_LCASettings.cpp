#include "VICUS_LcaSettings.h"
#include "VICUS_KeywordList.h"



void VICUS::LcaSettings::initDefaults() {
	m_para[P_TimePeriod].set("TimePeriod", 50, IBK::Unit("a"));
	m_para[P_PriceIncrease].set("PriceIncrease", 5, IBK::Unit("%"));
	m_para[P_FactorBnbSimpleMode].set("FactorBnbSimpleMode", 1.2, IBK::Unit("-"));
	m_para[P_NetUsageArea].set("NetUsageArea", 0.1, IBK::Unit("m2"));

	for(unsigned int i=0; i<NUM_M; ++i)
		m_flags[i].set(KeywordList::Keyword("LcaSettings::Module", i), false);

	m_lcaCalculationMode = LcaCalculationMode::CM_Simple;
	m_lcaCertificationSystem = LcaCertificationSytem::CS_BNB;

}
