#include "VICUS_LccSettings.h"



void VICUS::LccSettings::initDefaults() {
	m_para[P_DiscountingInterestRate].set("CalculationInterestRate", 1.5, IBK::Unit("%"));
    m_para[P_PriceIncreaseGeneral].set("PriceIncreaseGeneral", 2, IBK::Unit("%"));
	m_para[P_PriceIncreaseEnergy].set("PriceIncreaseEnergy", 5, IBK::Unit("%"));

	m_para[P_CoalConsumption].set("CoalConsumption", 1000, IBK::Unit("kWh/a"));
	m_para[P_ElectricityConsumption].set("ElectricityConsumption", 1000, IBK::Unit("kWh/a"));
	m_para[P_GasConsumption].set("GasConsumption", 1000, IBK::Unit("kWh/a"));

	m_intPara[IP_CoalPrice].set("CoalPrice", 16);
	m_intPara[IP_ElectricityPrice].set("ElectricityPrice", 42);
	m_intPara[IP_GasPrice].set("GasPrice", 15);
}
