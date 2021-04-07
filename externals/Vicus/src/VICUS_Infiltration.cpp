#include "VICUS_Infiltration.h"

#include "VICUS_KeywordList.h"

namespace VICUS {



bool Infiltration::isValid() const
{
	///TODO DIRK/KATJA
	if(m_id == INVALID_ID)
		return false;

	if(m_airChangeType == NUM_AC)
		return false;

	if(m_managementScheduleId == INVALID_ID)
		return false;
	else{
		/// TODO Dirk->Andreas wie komm ich jetzt an die Schedule Datenbank und kann die vorgegebene ID prüfen
		/// ob dieser Schedule valide ist?
	}
	try {
		m_para->checkedValue(VICUS::KeywordList::Keyword("Infiltration::para_t", P_AirChangeRate),
							 "1/h", "1/h", 0, true, 100, true, nullptr);

	}  catch (...) {
		return false;
	}

	if(m_airChangeType == AC_n50){
		try {
			m_para[P_ShieldingCoefficient].checkedValue(VICUS::KeywordList::Keyword("Infiltration::para_t", P_ShieldingCoefficient),
														"-", "-", 0, true, 10, true, nullptr);

		}  catch (...) {
			return false;
		}
	}

	return true;
}


}
