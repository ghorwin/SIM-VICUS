#ifndef VICUS_DailyCycleH
#define VICUS_DailyCycleH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"

#include <vector>

namespace VICUS {

/*! Defines time points and values.
*/
class DailyCycle {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMP(DailyCycle)

	/*! Checks if all referenced materials exist and if their parameters are valid. */
	bool isValid() const;

	DailyCycle multiply(const DailyCycle &other) const;

	DailyCycle operator *(const DailyCycle &other) const {return multiply(other);}

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Holds one or more day types from NANDRAD::Schedule::type_t enum.
		Only the days (Mo, ... , Fr, .. So) and ST_HOLIDAY.
	*/
	std::vector<int>		m_dayTypes;																// XML:E

	/*! Vector with time points. First value must always be 0.
		TODO Andreas welches Zeitformat hatte wir definiert? Stunden Minuten Sekunden?
	*/
	std::vector<double>		m_timePoints;															// XML:E

	/*! Vector with corresponding values (same size as m_timePoints vector).
		When data is interpreted as linearly interpolated, the values are taken as given directly
		at the matching time points. Otherwise, the value is taken constant during the interval starting
		at the corresponding time point.
	*/
	std::vector<double>		m_values;																// XML:E
};

} // namespace VICUS


#endif // VICUS_DailyCycleH
