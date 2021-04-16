#include "NANDRAD_NaturalVentilationModel.h"

#include "NANDRAD_KeywordList.h"

namespace NANDRAD {

void NaturalVentilationModel::checkParameters() const {

	// check for mandatory and required parameters
	// check for meaningful value ranges

	int enumVar = P_VentilationRate;
	m_para[enumVar].checkedValue(NANDRAD::KeywordList::Keyword("NaturalVentilationModel::m_para", enumVar),
								 "1/h", "1/h", 0.0, true, 100, true,
								   " 0 1/h <= Ventilation rate <= 100 1/h.");
	enumVar = P_MaximumRoomAirTemperatureACRLimit;
	m_para[enumVar].checkedValue(NANDRAD::KeywordList::Keyword("NaturalVentilationModel::m_para", enumVar),
								 "C", "C", -100, true, 100, true,
								 " -100 C <= Maximum room temperature <= 100 C.");
	enumVar = P_MinimumRoomAirTemperatureACRLimit;
	m_para[enumVar].checkedValue(NANDRAD::KeywordList::Keyword("NaturalVentilationModel::m_para", enumVar),
								 "C", "C", -100, true, 100, true,
								 " -100 C <= Minimum room temperature <= 100 C.");
	enumVar = P_MaximumEnviromentAirTemperatureACRLimit;
	m_para[enumVar].checkedValue(NANDRAD::KeywordList::Keyword("NaturalVentilationModel::m_para", enumVar),
								 "C", "C", -100, true, 100, true,
								 " -100 C <= Maximum enviroment temperature <= 100 C.");
	enumVar = P_MinimumEnviromentAirTemperatureACRLimit;
	m_para[enumVar].checkedValue(NANDRAD::KeywordList::Keyword("NaturalVentilationModel::m_para", enumVar),
								 "C", "C", -100, true, 100, true,
								 " -100 C <= Minimum enviroment temperature <= 100 C.");
	enumVar = P_DeltaTemperatureACRLimit;
	///TODO Dirk->Andreas geht das mit -100 K???
	m_para[enumVar].checkedValue(NANDRAD::KeywordList::Keyword("NaturalVentilationModel::m_para", enumVar),
								 "K", "K", -100, true, 100, true,
								 " -100 K <= Temperature differenz of room air temperature and enviroment temperature <= 100 K.");
	enumVar = P_WindSpeedACRLimit;
	m_para[enumVar].checkedValue(NANDRAD::KeywordList::Keyword("NaturalVentilationModel::m_para", enumVar),
								 "m/s", "m/s", 0, true, 40, true,
								 " 0 m/s <= Maximum wind speed <= 40 m/s.");

}


} // namespace NANDRAD

