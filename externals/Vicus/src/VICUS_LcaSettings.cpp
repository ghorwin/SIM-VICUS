#include "VICUS_LcaSettings.h"
#include "VICUS_KeywordList.h"



void VICUS::LcaSettings::initDefaults() {
    m_para[P_TimePeriod].set("TimePeriod", 50, IBK::Unit("a"));
	m_para[P_FactorBnbSimpleMode].set("FactorBnbSimpleMode", 1.2, IBK::Unit("-"));
	m_para[P_NetUsageArea].set("NetUsageArea", 0.1, IBK::Unit("m2"));

	for(unsigned int i=0; i<NUM_M; ++i)
		m_flags[i].set(KeywordList::Keyword("LcaSettings::Module", i), false);

	m_calculationMode = CalculationMode::CM_Simple;
	m_certificationSystem = CertificationSytem::CS_BNB;

	m_certificationModules = CertificationModules::CT_BNB;
}

bool VICUS::LcaSettings::isLcaCategoryDefined(EpdCategoryDataset::Module mod) const {
	switch (mod) {
		case EpdCategoryDataset::M_A1:	return m_certificationModules & M_A1;
		case EpdCategoryDataset::M_A2:	return m_certificationModules & M_A2;
		case EpdCategoryDataset::M_A3:	return m_certificationModules & M_A3;
		case EpdCategoryDataset::M_A4:	return m_certificationModules & M_A4;
		case EpdCategoryDataset::M_A5:	return m_certificationModules & M_A5;
		case EpdCategoryDataset::M_B1:	return m_certificationModules & M_B1;
		case EpdCategoryDataset::M_B2:	return m_certificationModules & M_B2;
		case EpdCategoryDataset::M_B3:	return m_certificationModules & M_B3;
		case EpdCategoryDataset::M_B4:	return m_certificationModules & M_B4;
		case EpdCategoryDataset::M_B5:	return m_certificationModules & M_B5;
		case EpdCategoryDataset::M_B6:	return m_certificationModules & M_B6;
		case EpdCategoryDataset::M_B7:	return m_certificationModules & M_B7;
		case EpdCategoryDataset::M_C1:	return m_certificationModules & M_C1;
		case EpdCategoryDataset::M_C2:	return m_certificationModules & M_C2;
		case EpdCategoryDataset::M_C3:	return m_certificationModules & M_C3;
		case EpdCategoryDataset::M_C4:	return m_certificationModules & M_C4;
		case EpdCategoryDataset::M_D:	return m_certificationModules & M_D;
		case EpdCategoryDataset::NUM_M: return m_certificationModules & NUM_M;
	}
}
