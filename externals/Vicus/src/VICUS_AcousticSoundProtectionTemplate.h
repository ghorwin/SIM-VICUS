#ifndef VICUS_SoundProtectionTemplateH
#define VICUS_SoundProtectionTemplateH

#include "VICUS_AbstractDBElement.h"
#include "VICUS_CodeGenMacros.h"

namespace VICUS {

class AcousticSoundProtectionTemplate : public AbstractDBElement {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE_OVERRIDE
	VICUS_COMPARE_WITH_ID

	/*! Building type, defined here and in VICUS Room! */
	enum AcousticBuildingType {
				   ABT_MultiFamilyHouse,	// Keyword: MultiFamilyHouse		'Wohngebäude und Gebäude mit Wohn- und Arbeitsbereichen'
				   ABT_Hotel,				// Keyword: Hotel					'Hotels und Beherbergungsstätten'
				   ABT_Hospital,			// Keyword: Hospital				'Krankenhäuser und Sanatorien'
				   ABT_School,				// Keyword: School					'Schulen und vergleichbaren Einrichtungen'
				   ABT_SingleFamilyHouse,	// Keyword: SingleFamilyHouse		'Einfamilien-Reihenhäusern und zwischen Doppelhäusern'
				   ABT_Office,				// Keyword: Office					'Büro'
				   NUM_ABT
				   // changes need to be done here and in Vicus Room!!!
	};

	/*! C'tor */
	AcousticSoundProtectionTemplate() {}

	/*! Destructor */
	// ~SoundProtectionTemplate() = default;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;

	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int					m_id = INVALID_ID;		// XML:A:required
	//:inherited	IBK::MultiLanguageString		m_displayName;			// XML:A
	//:inherited	QColor							m_color;				// XML:A

	/*! Notes. */
	IBK::MultiLanguageString						m_note;					// XML:E

	AcousticBuildingType							m_buildingType;			// XML:A
	std::vector<unsigned int>						m_idsTemplate;			// XML:E


};
} // namespace VICUS


#endif // VICUS_SoundProtectionTemplateH
