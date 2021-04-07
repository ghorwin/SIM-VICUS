#include "VICUS_VentilationNatural.h"
#include "VICUS_KeywordList.h"

namespace VICUS {


bool VentilationNatural::isValid() const
{
	///TODO DIRK/KATJA

	// is id valid?
	if ( m_id == INVALID_ID )
		return false;

	// is a schedule ID set?
	if ( m_scheduleId == INVALID_ID )
		return false;
	else {
	/// TODO Check Schedule ID
	/// we have to check also if the schedule with the specified ID exists!
	}

	try {
		// check whether a parameter with the correct unit has been set
		m_para[P_AirChangeRate].checkedValue(VICUS::KeywordList::Keyword("VentilationNatural::para_t", P_AirChangeRate),
											 "1/h", "1/h", 0, true, 100, true, nullptr);
	}  catch (...) {
		return false;
	}

	return true;
}


}
