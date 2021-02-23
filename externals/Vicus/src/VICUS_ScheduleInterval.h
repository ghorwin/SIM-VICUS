#ifndef VICUS_ScheduleIntervalH
#define VICUS_ScheduleIntervalH

#include <IBK_MultiLanguageString.h>

#include "VICUS_CodeGenMacros.h"

#include <vector>

#include "VICUS_DailyCycle.h"

namespace VICUS {

/*! Describes a period within a schedule where course of scheduled quantities is defined through
	sets of daily cycles.
*/
class ScheduleInterval {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE

	/*! Checks if all referenced materials exist and if their parameters are valid. */
	bool isValid() const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Display name of period. */
	IBK::MultiLanguageString	m_displayName;									// XML:A


	/*! Day index (0) where schedule period starts (max. 364). First period always starts with 0.
		Each period lasts until begin of next interval, or until end of year (if it is the last period).
	*/
	unsigned int				m_intervalStartDay=0;							// XML:A

	/*! Vector with daily cycles. */
	std::vector<DailyCycle>		m_dailyCycles;									// XML:E
};

} // namespace VICUS


#endif // VICUS_ScheduleIntervalH
