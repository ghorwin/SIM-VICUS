#include "VICUS_LCASettings.h"

#include "VICUS_KeywordList.h"


void VICUS::LCASettings::initDefaults() {
	m_para[P_TimePeriod].set("TimePeriod", 50, IBK::Unit("a"));
	m_para[P_PriceIncrease].set("PriceIncrease", 5, IBK::Unit("%"));
}
