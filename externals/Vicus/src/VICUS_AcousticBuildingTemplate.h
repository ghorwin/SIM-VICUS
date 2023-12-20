#ifndef VICUS_AcousticBuildingTemplateH
#define VICUS_AcousticBuildingTemplateH

#include "VICUS_AbstractDBElement.h"
#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"



namespace VICUS {
class AcousticBuildingTemplate : public AbstractDBElement {
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

public:

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;

	// *** PUBLIC MEMBER VARIABLES ***
	//:inherited	unsigned int					m_id = INVALID_ID;		// XML:A:required
	//:inherited	IBK::MultiLanguageString		m_displayName;			// XML:A

	AcousticBuildingType			m_buildingType;							// XML:A
	std::vector<unsigned int>		m_idsSoundProtectionTemplate;			// XML:E


};
} // namespace VICUS


#endif // VICUS_AcousticBuildingTemplateH
