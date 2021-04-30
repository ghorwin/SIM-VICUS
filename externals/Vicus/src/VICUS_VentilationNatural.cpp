#include "VICUS_VentilationNatural.h"
#include "VICUS_KeywordList.h"

namespace VICUS {


bool VentilationNatural::isValid() const {
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

AbstractDBElement::ComparisonResult VentilationNatural::equal(const AbstractDBElement *other) const {
	const VentilationNatural * otherVent = dynamic_cast<const VentilationNatural*>(other);
	if (otherVent == nullptr)
		return Different;

	//first check critical data

	//check parameters
	for(unsigned int i=0; i<NUM_P; ++i){
		if(m_para[i] != otherVent->m_para[i])
			return Different;
	}
	if(m_scheduleId != otherVent->m_scheduleId)
		return Different;

	//check meta data

	if(m_displayName != otherVent->m_displayName ||
			m_color != otherVent->m_color ||
			m_dataSource != otherVent->m_dataSource ||
			m_notes != otherVent->m_notes)
		return OnlyMetaDataDiffers;

	return Equal;
}


}
