#ifndef VICUS_AcousticReferenceComponentH
#define VICUS_AcousticReferenceComponentH

#include "VICUS_AbstractDBElement.h"
#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_Component.h"

#include <IBK_Parameter.h>

namespace VICUS {


class AcousticReferenceComponent : public AbstractDBElement {

public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE_OVERRIDE
	VICUS_COMPARE_WITH_ID

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;


	enum RequirementType	{
		RT_Basic,			// Keyword: basic		'Normal sound requirements'
		RT_Advanced,		// Keyword: advanced	'Enhanced sound requirements'
		NUM_RT

	};

	enum ComponentType		{
		CT_Ceiling,			// Keyword: Ceiling
		CT_Wall,			// Keyword: Wall
		CT_Door,			// Keyword: Door
		CT_Stairs,			// Keyword: Stairs
		NUM_CT
	};

	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int					m_id = INVALID_ID;		// XML:A:required
	//:inherited	IBK::MultiLanguageString		m_displayName;			// XML:A
	//:inherited	QColor							m_color;				// XML:A

	/*! Describes the acoustic type of the building*/
	IDType					m_buildingType = VICUS::INVALID_ID;			// XML:A

	RequirementType			m_requirementType = NUM_RT;					// XML:A

	ComponentType			m_type = NUM_CT;							// XML:E

	double					m_impactSoundOneStructureUnit = -1;			// XML:E
	double					m_impactSoundDifferentStructureUnit = -1;	// XML:E

	double					m_airborneSoundOneStructureUnit = -1;		// XML:E
	double					m_airborneSoundDifferentStructureUnit = -1;	// XML:E

	unsigned int			m_idAcousticTemplateA = VICUS::INVALID_ID;	// XML:E
	unsigned int			m_idAcousticTemplateB = VICUS::INVALID_ID;	// XML:E


};

} // namespace VICUS

#endif // VICUS_AcousticReferenceComponentH
