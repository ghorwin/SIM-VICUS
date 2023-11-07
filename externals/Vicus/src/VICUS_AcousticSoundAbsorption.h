#ifndef VICUS_AcousticSoundAbsorptionH
#define VICUS_AcousticSoundAbsorptionH

#include "VICUS_AbstractDBElement.h"
#include "VICUS_CodeGenMacros.h"

namespace VICUS {

class AcousticSoundAbsorption : public AbstractDBElement
{
public:
	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE_PRIVATE
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

	void readXML(const TiXmlElement * element);
	TiXmlElement * writeXML(TiXmlElement * parent) const;

	/*! Checks if all parameters are valid. */
	bool isValid() const;

	// AbstractDBElement interface
	ComparisonResult equal(const AbstractDBElement *other) const;

	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int					m_id = INVALID_ID;		// XML:A:required
	//:inherited	IBK::MultiLanguageString		m_displayName;			// XML:A

	double						m_soundAbsorption[NUM_SF] = {0, 0, 0, 0, 0, 0};
};

} // namespace VICUS

#endif // VICUS_AcousticSoundAbsorptionH
