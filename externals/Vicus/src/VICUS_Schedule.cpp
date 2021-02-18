#include "VICUS_Schedule.h"

namespace VICUS {

bool Schedule::isValid() const {

	// check that the period start days are increasing
	/// TODO Andreas: Warum muss das sein? Wir brauchen doch nur prüfen ob keine doppelten vorhanden sind. Sortierung ist doch egal


	//Wie fangen wir ab wenn es einen annual und ein Schedule Interval gibt?
	//was gilt dann?
	//zudem gibs im annual nochmal eine Interpolation method auch die müsste dann überschrieben bzw. geprüft werden

	//for ScheduleInterval

	std::set<unsigned int>	periodStart;
	for(unsigned int i=0; i<m_periods.size(); ++i){
		periodStart.insert(m_periods[i].m_intervalStartDay);
		if(periodStart.size() != i)
			return false;	//period exist already
		if(!m_periods[i].isValid())
			return false;
	}

	return true;
}


} // namespace VICUS
