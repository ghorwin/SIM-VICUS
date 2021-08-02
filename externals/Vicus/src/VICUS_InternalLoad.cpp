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
	if(m_id == INVALID_ID)
		return false;

	switch (m_category) {
		case VICUS::InternalLoad::IC_Person:{

			if(m_personCountMethod == NUM_PCM)
				return false;

			try {
				switch(m_personCountMethod){
					case InternalLoad::PCM_PersonPerArea:{
						m_para[P_PersonPerArea].checkedValue(KeywordList::Keyword("InternalLoad::para_t", P_PersonPerArea),
														   "Person/m2", "Person/m2", 0, true, 100000, true, nullptr);
					}
					break;
					case InternalLoad::PCM_AreaPerPerson:{
						m_para[P_AreaPerPerson].checkedValue(KeywordList::Keyword("InternalLoad::para_t", P_AreaPerPerson),
														   "m2/Person", "m2/Person", 0, false, 100000, true, nullptr);

					}
					break;
					case InternalLoad::PCM_PersonCount:{
						m_para[P_PersonCount].checkedValue(KeywordList::Keyword("InternalLoad::para_t", P_PersonCount),
														   "-", "-", 0, true, 100000, true, nullptr);
					}
					break;
					case InternalLoad::NUM_PCM:
						return false;
				}

			}  catch (...) {
				return false;
			}

			if(m_activityScheduleId == INVALID_ID || m_occupancyScheduleId == INVALID_ID)
				return false;

			//check schedules occ and act
			// check if schedule ID is existing and valid
			const Schedule * actSched = scheduleDB[m_activityScheduleId];
			if (actSched == nullptr)
				return false;
			if (!actSched->isValid())
				return false;

			const Schedule * occSched = scheduleDB[m_occupancyScheduleId];
			if (occSched == nullptr)
				return false;
			if (!occSched->isValid())
				return false;

		}
		break;
		case VICUS::InternalLoad::IC_ElectricEquiment:
		case VICUS::InternalLoad::IC_Lighting:
		case VICUS::InternalLoad::IC_Other:{
			switch (m_powerMethod) {
				case VICUS::InternalLoad::PM_PowerPerArea: {
					try {
						m_para[P_PowerPerArea].checkedValue(KeywordList::Keyword("InternalLoad::para_t", P_PowerPerArea),
															   "W/m2", "W/m2", 0, true, 1000, true, nullptr);
					}  catch (...) {
						return false;
					}
				}
				break;
				case VICUS::InternalLoad::PM_Power:{
					try {
						m_para[P_Power].checkedValue(KeywordList::Keyword("InternalLoad::para_t", P_Power),
															   "W", "W", 0, true, 100000, true, nullptr);
					}  catch (...) {
						return false;
					}
				}
				break;
				case VICUS::InternalLoad::NUM_PM:	return false;
			}

			if(m_powerManagementScheduleId == INVALID_ID)
				return false;

			// check if schedule ID is existing and valid
			const Schedule * powerSched = scheduleDB[m_powerManagementScheduleId];
			if (powerSched == nullptr)
				return false;
			if (!powerSched->isValid())
				return false;
		}
		break;

		case NUM_MC:	return false;
	}

	try {

		m_para[P_ConvectiveHeatFactor].checkedValue(KeywordList::Keyword("InternalLoad::para_t", P_ConvectiveHeatFactor),
											   "---", "---", 0, true, 1, true, nullptr);
		if(m_category == IC_ElectricEquiment){
			m_para[P_LossHeatFactor].checkedValue(KeywordList::Keyword("InternalLoad::para_t", P_LossHeatFactor),
												   "---", "---", 0, true, 1, true, nullptr);
			m_para[P_LatentHeatFactor].checkedValue(KeywordList::Keyword("InternalLoad::para_t", P_LatentHeatFactor),
												   "---", "---", 0, true, 1, true, nullptr);
		}
	}  catch (...) {
		return false;
	}

	return true;
}

AbstractDBElement::ComparisonResult InternalLoad::equal(const AbstractDBElement *other) const {
	const InternalLoad * otherIntLoad = dynamic_cast<const InternalLoad*>(other);
	if (otherIntLoad == nullptr)
		return Different;

	//first check critical data

	//check parameters
	for(unsigned int i=0; i<NUM_P; ++i){
		if(m_para[i] != otherIntLoad->m_para[i])
			return Different;
	}
	if(m_category != otherIntLoad->m_category ||
			m_personCountMethod != otherIntLoad->m_personCountMethod||
			m_powerMethod != otherIntLoad->m_powerMethod ||
			m_occupancyScheduleId != otherIntLoad->m_occupancyScheduleId ||
			m_activityScheduleId != otherIntLoad->m_activityScheduleId ||
			m_powerManagementScheduleId != otherIntLoad->m_powerManagementScheduleId)
		return Different;

	//check meta data

	if(m_displayName != otherIntLoad->m_displayName ||
			m_color != otherIntLoad->m_color)
		return OnlyMetaDataDiffers;

	return Equal;
}


}
