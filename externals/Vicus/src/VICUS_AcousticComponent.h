 #ifndef VICUS_AcousticComponentH
#define VICUS_AcousticComponentH

#include "VICUS_AbstractDBElement.h"
#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include <IBK_Parameter.h>



namespace VICUS {

class AcousticComponent : public AbstractDBElement {
public:


	/*! Basic parameters. */
	enum para_t {
		/*! Area. */
		P_Area,							// Keyword: Area							[m2]	'Area'
		/*! Fraction. */
		P_Fraction,						// Keyword: Fraction						[%]		'Fraction'
		NUM_P
	};

	// TODO Anton: insert the right descriptions
	/*! The description is used to identify the unit in the gui for all other equipment (electric, lights, other, ...). */
	enum SizeMethod {
		SM_AreaTotal,					// Keyword: AreaTotal					[m2]		'Total Area'
		SM_AreaFraction,				// Keyword: AreaFraction				[-]			'Fraction of Area'
		NUM_SM
	};

	/*! Frequencies. */
	enum ReverberationFrequency {
		RF_125Hz,
		RF_250Hz,
		RF_500Hz,
		RF_1000Hz,
		RF_2000Hz,
		RF_4000Hz,
		NUM_RF
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE_OVERRIDE
	VICUS_COMPARE_WITH_ID

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;

	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int					m_id = INVALID_ID;		// XML:A:required
	//:inherited	IBK::MultiLanguageString		m_displayName;			// XML:A
	//:inherited	QColor							m_color;				// XML:A

	/*! stores the impact sound value. */
	double						m_impactSoundValue;					// XML:E

	/*! stores the air sound resistence value. */
	double						m_airSoundResistenceValue;			// XML:E

	//TODO: Ask if there should be a default method assigned
	/*! stores the method for the reverberation calculation */
	SizeMethod					m_sizeMethod = SM_AreaFraction;		// XML:A

	/*! Frequency depending reverberation time. */
	double						m_reverberationTime[NUM_RF] = {0, 0, 0, 0, 0, 0};

	/*! List of parameters. */
	IBK::Parameter				m_para[NUM_P];						// XML:E

};

} // namespace VICUS

#endif // VICUS_AcousticComponentH
