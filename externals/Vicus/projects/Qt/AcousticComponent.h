#ifndef AcousticComponentH
#define AcousticComponentH

#include "VICUS_AbstractDBElement.h"
#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"

namespace VICUS {

class AcousticComponent : public AbstractDBElement {
public:
	AcousticComponent();

	VICUS_READWRITE_OVERRIDE
	VICUS_COMPARE_WITH_ID

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;

	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int					m_id = INVALID_ID;		// XML:A:required
	//:inherited	IBK::MultiLanguageString		m_displayName;			// XML:A

	/*! stores the impact sound value. */
	double						m_impactSoundValue;					// XML:E

	/*! stores the air sound resistence value. */
	double						m_airSoundRestistenceValue;			// XML:E

};

} // namespace VICUS

#endif // AcousticComponentH
