#include "VICUS_ZoneControlThermostat.h"

#include "VICUS_KeywordList.h"

namespace VICUS {


bool ZoneControlThermostat::isValid() const
{
	if(m_id == INVALID_ID)
		return false;

	if(m_ctrlVal == NUM_CV)
		return false;

	if(m_coolingSetpointScheduleId == INVALID_ID || m_heatingSetpointScheduleId == INVALID_ID)
		return false;
	else{
		/// TODO Dirk->Andreas wie komm ich jetzt an die Schedule Datenbank und kann die vorgegebene ID prüfen
		/// ob dieser Schedule valide ist?
	}
	try {
		m_para[P_ToleranceCooling].checkedValue(KeywordList::Keyword("ZoneControlThermostat::para_t", P_ToleranceCooling),
							 "K", "K", 0, true, 5, true, nullptr);
		m_para[P_ToleranceHeating].checkedValue(KeywordList::Keyword("ZoneControlThermostat::para_t", P_ToleranceHeating),
							 "K", "K", 0, true, 5, true, nullptr);

	}  catch (...) {
		return false;
	}

	return true;
}


}
