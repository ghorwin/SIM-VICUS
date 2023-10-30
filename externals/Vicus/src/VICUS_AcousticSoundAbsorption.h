#ifndef VICUS_SOUNDABSORPTION_H
#define VICUS_SOUNDABSORPTION_H

#include "VICUS_AbstractDBElement.h"
#include "VICUS_CodeGenMacros.h"

namespace VICUS {

class SoundAbsorption : public AbstractDBElement
{
public:
	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE_OVERRIDE
	VICUS_COMPARE_WITH_ID

	/*! Frequencies. */
	enum SoundAbsorptionFrequency {
		SF_125Hz,
		SF_250Hz,
		SF_500Hz,
		SF_1000Hz,
		SF_2000Hz,
		SF_4000Hz,
		NUM_SF
	};

	double						m_soundAbsorption[NUM_SF] = {0, 0, 0, 0, 0, 0};			// XML:E

	// AbstractDBElement interface
	ComparisonResult equal(const AbstractDBElement *other) const;
};

} // namespace VICUS

#endif // VICUS_SOUNDABSORPTION_H
