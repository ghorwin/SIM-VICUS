#include "VICUS_InternalLoad.h"

#include "VICUS_KeywordList.h"

namespace VICUS {


bool InternalLoad::isValid() const
{
	///TODO DIRK/KATJA
	if(m_id == INVALID_ID)
		return false;

	switch (m_category) {
		case VICUS::InternalLoad::IC_Person:{

			if(m_personCountMethod == NUM_PCM)
				return false;

			///TODO Dirk->Andreas Einheiten anpassen und neu aufnehmen in Liste
			/// wie machen wir das?
			try {
				m_para[P_PersonCount].checkedValue(KeywordList::Keyword("InternalLoad::para_t", P_PersonCount),
												   "-", "-", 0, true, 100000, true, nullptr);
				m_para[P_PersonPerArea].checkedValue(KeywordList::Keyword("InternalLoad::para_t", P_PersonPerArea),
												   "m2", "m2", 0, true, 100000, true, nullptr);
				m_para[P_AreaPerPerson].checkedValue(KeywordList::Keyword("InternalLoad::para_t", P_AreaPerPerson),
												   "m2", "m2", 0, true, 100000, true, nullptr);

			}  catch (...) {
				return false;
			}

			if(m_activityScheduleId == INVALID_ID || m_occupancyScheduleId == INVALID_ID)
				return false;

			//check schedules occ and act
			/// TODO Dirk->Andreas wie komm ich jetzt an die Schedule Datenbank und kann die vorgegebene ID prüfen
			/// ob dieser Schedule valide ist?
		}
		break;
		case VICUS::InternalLoad::IC_ElectricEquiment:
		case VICUS::InternalLoad::IC_Lighting:
		case VICUS::InternalLoad::IC_Other:{
			switch (m_powerMethod) {
				case VICUS::InternalLoad::PM_PowerPerArea: {
					try {
						///TODO Dirk->Andreas Einheiten anpassen und neu aufnehmen in Liste
						m_para[P_PowerPerArea].checkedValue(KeywordList::Keyword("InternalLoad::para_t", P_PowerPerArea),
															   "W/m2", "W/m2", 0, true, 1000, true, nullptr);
					}  catch (...) {
						return false;
					}
				}
				break;
				case VICUS::InternalLoad::PM_Power:{
					try {
						///TODO Dirk->Andreas Einheiten anpassen und neu aufnehmen in Liste
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

			//check schedules occ and act
			/// TODO Dirk->Andreas wie komm ich jetzt an die Schedule Datenbank und kann die vorgegebene ID prüfen
			/// ob dieser Schedule valide ist?
		}
		break;

		case NUM_MC:	return false;
	}

	try {

		m_para[P_ConvectiveHeatFactor].checkedValue(KeywordList::Keyword("InternalLoad::para_t", P_ConvectiveHeatFactor),
											   "---", "---", 0, true, 1, true, nullptr);
		if(m_category != IC_Person){
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


}
