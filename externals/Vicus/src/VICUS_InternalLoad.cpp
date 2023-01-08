/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

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

#include "VICUS_InternalLoad.h"

#include "VICUS_KeywordList.h"

namespace VICUS {

bool InternalLoad::isValid(const Database<Schedule> &scheduleDB) const {
	std::string err;
	switch (m_category) {
		case VICUS::InternalLoad::IC_Person: {

			try {
				switch(m_personCountMethod) {
					case InternalLoad::PCM_PersonPerArea: {
						m_para[P_PersonPerArea].checkedValue(KeywordList::Keyword("InternalLoad::para_t", P_PersonPerArea),
														   "Person/m2", "Person/m2", 0, true, 100000, true, nullptr);
					}
					break;
					case InternalLoad::PCM_AreaPerPerson: {
						m_para[P_AreaPerPerson].checkedValue(KeywordList::Keyword("InternalLoad::para_t", P_AreaPerPerson),
														   "m2/Person", "m2/Person", 0, false, 100000, true, nullptr);

					}
					break;
					case InternalLoad::PCM_PersonCount: {
						m_para[P_PersonCount].checkedValue(KeywordList::Keyword("InternalLoad::para_t", P_PersonCount),
														   "-", "-", 0, true, 100000, true, nullptr);
					}
					break;
					case InternalLoad::NUM_PCM:
						return false;
				}

			}  catch (IBK::Exception &ex) {
				m_errorMsg = ex.what();
				return false;
			}

			if (m_idActivitySchedule == INVALID_ID) {
				m_errorMsg = "Activity schedule is not set.";
				return false;
			}
			if (m_idOccupancySchedule == INVALID_ID) {
				m_errorMsg = "Occupancy schedule is not set.";
				return false;
			}

			// check schedules occ and act
			// check if schedule ID is existing and valid
			const Schedule * actSched = scheduleDB[m_idActivitySchedule];
			if (actSched == nullptr) {
				m_errorMsg = "Activity schedule with id '" + std::to_string(m_idActivitySchedule) + "' does not exist.";
				return false;
			}
			if (!actSched->isValid(err, true)) {
				m_errorMsg = "Activity schedule '" + actSched->m_displayName.string("de", true) + "' is invalid.";
				return false;
			}

			// check schedule moist rate
			if (m_idMoistureProductionRatePerAreaSchedule != INVALID_ID) {
				const Schedule *moistSched = scheduleDB[m_idMoistureProductionRatePerAreaSchedule];
				if (moistSched == nullptr) {
					m_errorMsg = "Moisture rate schedule with id '" + std::to_string(m_idMoistureProductionRatePerAreaSchedule) + "' does not exist.";
					return false;
				}
				if (!moistSched->isValid(err, true)) {
					m_errorMsg = "Moisture rate schedule '" + moistSched->m_displayName.string("en", true) + "' is invalid.";
					return false;
				}
			}

			const Schedule * occSched = scheduleDB[m_idOccupancySchedule];
			if (occSched == nullptr) {
				m_errorMsg = "Occupancy schedule with id '" + std::to_string(m_idActivitySchedule) + "' does not exist.";
				return false;
			}
			if (!occSched->isValid(err, true)) {
				m_errorMsg = "Occupancy schedule '" + actSched->m_displayName.string("de", true) + "' is invalid.";
				return false;
			}
		}
		break;

		case VICUS::InternalLoad::IC_ElectricEquiment:
		case VICUS::InternalLoad::IC_Lighting:
		case VICUS::InternalLoad::IC_Other: {
			switch (m_powerMethod) {
				case VICUS::InternalLoad::PM_PowerPerArea: {
					try {
						m_para[P_PowerPerArea].checkedValue(KeywordList::Keyword("InternalLoad::para_t", P_PowerPerArea),
															"W/m2", "W/m2", 0, true, 1000, true, nullptr);
					}  catch (IBK::Exception &ex) {
						m_errorMsg = ex.what();
						return false;
					}
				}
				break;
				case VICUS::InternalLoad::PM_Power: {
					try {
						m_para[P_Power].checkedValue(KeywordList::Keyword("InternalLoad::para_t", P_Power),
													 "W", "W", 0, true, 100000, true, nullptr);
					}  catch (IBK::Exception &ex) {
						m_errorMsg = ex.what();
						return false;
					}
				}
				break;
				case VICUS::InternalLoad::NUM_PM: {
					m_errorMsg = "Internal loads type is not defined.";
					return false;
				}
			}

			if (m_idPowerManagementSchedule == INVALID_ID) {
				m_errorMsg = "Power management schedule is not set.";
				return false;
			}

			// check if schedule ID is existing and valid
			const Schedule * powerSched = scheduleDB[m_idPowerManagementSchedule];
			if (powerSched == nullptr) {
				m_errorMsg = "Power management schedule with id '" + std::to_string(m_idPowerManagementSchedule) + "' does not exist.";
				return false;
			}
			if (!powerSched->isValid(err, true)) {
				m_errorMsg = "Power management '" + powerSched->m_displayName.string("de", true) + "' is invalid.";
				return false;
			}
		}
		break;

		case NUM_MC: {
			m_errorMsg = "Internal load category is not set.";
			return false;
		}
	}

	try {

		m_para[P_ConvectiveHeatFactor].checkedValue(KeywordList::Keyword("InternalLoad::para_t", P_ConvectiveHeatFactor),
													"---", "---", 0, true, 1, true, nullptr);
		if (m_category == IC_ElectricEquiment) {
			m_para[P_LossHeatFactor].checkedValue(KeywordList::Keyword("InternalLoad::para_t", P_LossHeatFactor),
												  "---", "---", 0, true, 1, true, nullptr);
			m_para[P_LatentHeatFactor].checkedValue(KeywordList::Keyword("InternalLoad::para_t", P_LatentHeatFactor),
													"---", "---", 0, true, 1, true, nullptr);
		}
	}  catch (IBK::Exception &ex) {
		m_errorMsg = ex.what();
		return false;
	}

	return true;
}


AbstractDBElement::ComparisonResult InternalLoad::equal(const AbstractDBElement *other) const {
	const InternalLoad * otherIntLoad = dynamic_cast<const InternalLoad*>(other);
	if (otherIntLoad == nullptr)
		return Different;

	for (unsigned int i=0; i<NUM_P; ++i){
		if (m_para[i] != otherIntLoad->m_para[i])
			return Different;
	}
	if (	m_category != otherIntLoad->m_category ||
			m_personCountMethod != otherIntLoad->m_personCountMethod||
			m_powerMethod != otherIntLoad->m_powerMethod ||
			m_idOccupancySchedule != otherIntLoad->m_idOccupancySchedule ||
			m_idActivitySchedule != otherIntLoad->m_idActivitySchedule ||
			m_idMoistureProductionRatePerAreaSchedule != otherIntLoad->m_idMoistureProductionRatePerAreaSchedule ||
			m_idPowerManagementSchedule != otherIntLoad->m_idPowerManagementSchedule)
		return Different;

	//check meta data

	if (m_displayName != otherIntLoad->m_displayName)
		return OnlyMetaDataDiffers;

	return Equal;
}


} // namespace VICUS
