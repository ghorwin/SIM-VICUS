#include "VICUS_ZoneControlShading.h"

#include "VICUS_KeywordList.h"

namespace VICUS {


bool ZoneControlShading::isValid() const
{
	// is id valid?
	if ( m_id == INVALID_ID )
		return false;

	// check whether category is valid?
	switch (m_category) {
	case VICUS::ZoneControlShading::C_GlobalHorizontalSensor:{
		try {
			// check whether a parameter with the correct unit has been set
			m_para[P_GlobalHorizontal].checkedValue(KeywordList::Keyword("ZoneControlShading::para_t", P_GlobalHorizontal),
												 "W/m2", "W/m2", 0, true, 1500, true, nullptr);
			m_para[P_DeadBand].checkedValue(KeywordList::Keyword("ZoneControlShading::para_t", P_DeadBand),
												 "W/m2", "W/m2", 0, true, 1500, true, nullptr);
		}  catch (...) {
			return false;
		}
	}
	break;
	case VICUS::ZoneControlShading::C_GlobalHorizontalAndVerticalSensors:{
		try {
			// check whether a parameter with the correct unit has been set
			for (unsigned int i=0; i<NUM_P; ++i) {
				m_para[i].checkedValue(KeywordList::Keyword("ZoneControlShading::para_t", i),
													 "W/m2", "W/m2", 0, true, 1500, true, nullptr);
			}

		}  catch (...) {
			return false;
		}
	}
	break;
	case VICUS::ZoneControlShading::NUM_C:
		return false;

	}

	return true;
}


}
