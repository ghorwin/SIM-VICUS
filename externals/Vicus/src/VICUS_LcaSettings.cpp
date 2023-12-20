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

	m_idUsage[UT_Coal] = INVALID_ID;
	m_idUsage[UT_Electricity] = INVALID_ID;
	m_idUsage[UT_Gas] = INVALID_ID;
}

bool VICUS::LcaSettings::isLcaCategoryDefined(EpdModuleDataset::Module mod) const {
	switch (mod) {
		case EpdModuleDataset::M_A1:	return m_certificationModules & M_A1;
		case EpdModuleDataset::M_A2:	return m_certificationModules & M_A2;
		case EpdModuleDataset::M_A3:	return m_certificationModules & M_A3;
		case EpdModuleDataset::M_A4:	return m_certificationModules & M_A4;
		case EpdModuleDataset::M_A5:	return m_certificationModules & M_A5;
		case EpdModuleDataset::M_B1:	return m_certificationModules & M_B1;
		case EpdModuleDataset::M_B2:	return m_certificationModules & M_B2;
		case EpdModuleDataset::M_B3:	return m_certificationModules & M_B3;
		case EpdModuleDataset::M_B4:	return m_certificationModules & M_B4;
		case EpdModuleDataset::M_B5:	return m_certificationModules & M_B5;
		case EpdModuleDataset::M_B6:	return m_certificationModules & M_B6;
		case EpdModuleDataset::M_B7:	return m_certificationModules & M_B7;
		case EpdModuleDataset::M_C1:	return m_certificationModules & M_C1;
		case EpdModuleDataset::M_C2:	return m_certificationModules & M_C2;
		case EpdModuleDataset::M_C3:	return m_certificationModules & M_C3;
		case EpdModuleDataset::M_C4:	return m_certificationModules & M_C4;
		case EpdModuleDataset::M_D:	    return m_certificationModules & M_D;
		case EpdModuleDataset::NUM_M:   return m_certificationModules & NUM_M;
	}

	return false;
}

bool VICUS::LcaSettings::isLcaCategoryDefined(EpdModuleDataset::Module mod, CertificationModules modules) {
	switch (mod) {
		case EpdModuleDataset::M_A1:	return modules & M_A1;
		case EpdModuleDataset::M_A2:	return modules & M_A2;
		case EpdModuleDataset::M_A3:	return modules & M_A3;
		case EpdModuleDataset::M_A4:	return modules & M_A4;
		case EpdModuleDataset::M_A5:	return modules & M_A5;
		case EpdModuleDataset::M_B1:	return modules & M_B1;
		case EpdModuleDataset::M_B2:	return modules & M_B2;
		case EpdModuleDataset::M_B3:	return modules & M_B3;
		case EpdModuleDataset::M_B4:	return modules & M_B4;
		case EpdModuleDataset::M_B5:	return modules & M_B5;
		case EpdModuleDataset::M_B6:	return modules & M_B6;
		case EpdModuleDataset::M_B7:	return modules & M_B7;
		case EpdModuleDataset::M_C1:	return modules & M_C1;
		case EpdModuleDataset::M_C2:	return modules & M_C2;
		case EpdModuleDataset::M_C3:	return modules & M_C3;
		case EpdModuleDataset::M_C4:	return modules & M_C4;
		case EpdModuleDataset::M_D:	    return modules & M_D;
		case EpdModuleDataset::NUM_M:   return modules & NUM_M;
	}

	return false;
}
