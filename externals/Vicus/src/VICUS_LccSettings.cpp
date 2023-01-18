#include "VICUS_LccSettings.h"
#include "VICUS_KeywordList.h"



void VICUS::LccSettings::initDefaults() {
    m_para[P_CalculationInterestRate].set("CalculationInterestRate", 1.5, IBK::Unit("%"));
    m_para[P_PriceIncreaseGeneral].set("PriceIncreaseGeneral", 2, IBK::Unit("%"));
    m_para[P_PriceIncreaseGeneral].set("PriceIncreaseEnergy", 5, IBK::Unit("%"));

}
