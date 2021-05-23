/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "NANDRAD_NaturalVentilationModel.h"

#include "NANDRAD_KeywordList.h"

namespace NANDRAD {

void NaturalVentilationModel::checkParameters() const {

	// check for mandatory and required parameters
	// check for meaningful value ranges

	int enumVar = P_VentilationRate;
	switch (m_modelType) {
		case NANDRAD::NaturalVentilationModel::MT_Constant: {
			m_para[enumVar].checkedValue(NANDRAD::KeywordList::Keyword("NaturalVentilationModel::para_t", enumVar),
										 "1/h", "1/h", 0.0, true, 100, true,
										   " 0 1/h <= Ventilation rate <= 100 1/h.");
		} break;

		case NANDRAD::NaturalVentilationModel::MT_Scheduled:
			// nothing to check
		break;

		case NANDRAD::NaturalVentilationModel::MT_ScheduledWithBaseACR:{
			m_para[enumVar].checkedValue(NANDRAD::KeywordList::Keyword("NaturalVentilationModel::para_t", enumVar),
										 "1/h", "1/h", 0.0, true, 100, true,
										 " 0 1/h <= Ventilation rate <= 100 1/h.");
			enumVar = P_MaximumRoomAirTemperatureACRLimit;
			m_para[enumVar].checkedValue(NANDRAD::KeywordList::Keyword("NaturalVentilationModel::para_t", enumVar),
										 "C", "C", -100, true, 100, true,
										 " -100 C <= Maximum room temperature <= 100 C.");
			enumVar = P_MinimumRoomAirTemperatureACRLimit;
			m_para[enumVar].checkedValue(NANDRAD::KeywordList::Keyword("NaturalVentilationModel::para_t", enumVar),
										 "C", "C", -100, true, 100, true,
										 " -100 C <= Minimum room temperature <= 100 C.");
			enumVar = P_MaximumEnviromentAirTemperatureACRLimit;
			m_para[enumVar].checkedValue(NANDRAD::KeywordList::Keyword("NaturalVentilationModel::para_t", enumVar),
										 "C", "C", -100, true, 100, true,
										 " -100 C <= Maximum enviroment temperature <= 100 C.");
			enumVar = P_MinimumEnviromentAirTemperatureACRLimit;
			m_para[enumVar].checkedValue(NANDRAD::KeywordList::Keyword("NaturalVentilationModel::para_t", enumVar),
										 "C", "C", -100, true, 100, true,
										 " -100 C <= Minimum enviroment temperature <= 100 C.");
			enumVar = P_DeltaTemperatureACRLimit;
			m_para[enumVar].checkedValue(NANDRAD::KeywordList::Keyword("NaturalVentilationModel::para_t", enumVar),
										 "K", "K", -100, true, 100, true,
										 " -100 K <= Temperature differenz of room air temperature and enviroment temperature <= 100 K.");
			enumVar = P_WindSpeedACRLimit;
			m_para[enumVar].checkedValue(NANDRAD::KeywordList::Keyword("NaturalVentilationModel::para_t", enumVar),
										 "m/s", "m/s", 0, true, 40, true,
										 " 0 m/s <= Maximum wind speed <= 40 m/s.");
		} break;
		case NANDRAD::NaturalVentilationModel::NUM_MT: break;
	}

}


} // namespace NANDRAD

