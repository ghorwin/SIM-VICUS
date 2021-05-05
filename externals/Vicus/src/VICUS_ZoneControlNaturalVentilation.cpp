#include "VICUS_ZoneControlNaturalVentilation.h"
#include "VICUS_KeywordList.h"

namespace VICUS {


bool ZoneControlNaturalVentilation::isValid() const
{
	// we have to check if the object is valid

	// is id valid?
	if ( m_id == INVALID_ID )
		return false;

	for (unsigned int i = 0; i < NUM_ST; ++i) {
		// is a schedule ID set?
		if ( m_scheduleId[i] == INVALID_ID )
			return false;
		else {
		/// TODO Check Schedule ID
		/// we have to check also if the schedule with the specified ID exists!
		}

		try {
			switch (i) {
			case ST_TemperatureAirMax:
			case ST_TemperatureAirMin:
			case ST_TemperatureOutsideMin:
			case ST_TemperatureOutsideMax:
				// check whether a parameter with the correct unit has been set
				m_para[i].checkedValue(VICUS::KeywordList::Keyword("ZoneControlNaturalVentilation::ScheduleType", i),
													 "T", "T", -100, true, 100, true, nullptr);
			break;
			case ST_TemperatureDifference:
				// check whether a parameter with the correct unit has been set
				m_para[i].checkedValue(VICUS::KeywordList::Keyword("ZoneControlNaturalVentilation::ScheduleType", i),
													 "K", "K", -100, true, 100, true, nullptr);
			break;
			case ST_WindSpeedMax:
				// check whether a parameter with the correct unit has been set
				m_para[i].checkedValue(VICUS::KeywordList::Keyword("ZoneControlNaturalVentilation::ScheduleType", i),
													 "m/s", "m/s", 0, true, 40, true, nullptr);
			break;
			}
		}  catch (...) {
			return false;
		}
	}

	return true;

}

AbstractDBElement::ComparisonResult ZoneControlNaturalVentilation::equal(const AbstractDBElement *other) const {
	const ZoneControlNaturalVentilation * otherVent = dynamic_cast<const ZoneControlNaturalVentilation*>(other);
	if (otherVent == nullptr)
		return Different;

	//first check critical data

	//check parameters
	for(unsigned int i=0; i<NUM_ST; ++i){
		if(m_para[i] != otherVent->m_para[i])
			return Different;
	}

	for(unsigned int i=0; i<NUM_ST; ++i)
		if(m_scheduleId[i] != otherVent->m_scheduleId[i])
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
