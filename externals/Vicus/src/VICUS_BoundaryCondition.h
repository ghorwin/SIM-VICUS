#ifndef VICUS_BoundaryConditionH
#define VICUS_BoundaryConditionH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"

#include <QString>
#include <vector>

#include <IBK_Parameter.h>
#include <IBK_MultiLanguageString.h>

#include "VICUS_AbstractDBElement.h"

namespace VICUS {

class BoundaryCondition : public AbstractDBElement {
public:

	/*! Basic parameters. */
	enum para_t {
		//thermal paramters
		P_SolarAbsorption,			// Keyword: SolarAbsorption			[---]	'Solar absorption coefficient.'
		P_Emissivity,				// Keyword: Emissivity				[---]	'Constant Long wave emissivity.'
		//P_VisibleAbsorption,		// Keyword: VisibleAbsorption		[---]	'Visible absorption coefficient.'
		/*! 'Constant heat transfer coefficient [W/m2K].' */
		P_HeatTransferCoefficient,	// Keyword: HeatTransferCoefficient [W/m2K]		'Constant heat transfer coefficient.'
		NUM_P
	};

	enum HeatConductionModel{
		/*! Constant transfer coefficient. */
		MT_Constant,				// Keyword: Constant		'Constant model.'
		/// TODO Andreas : ist das hier so richtig falls das Modell detaillierter wird?
		/// Wie wird dann mit weiteren Parameter umgegangen die das Model benötigt
		//MT_WindVelocity,			// Keyword: WindVelocity	'Heat transfer coefficient depends on wind velocity.'
		NUM_MT
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of boundary condition. */
	unsigned int					m_id = INVALID_ID;		// XML:A:required

	/*! Display name of boundary condition. */
	IBK::MultiLanguageString		m_displayName;			// XML:A

	/*! List of parameters. */
	IBK::Parameter					m_para[NUM_P];			// XML:E


};

} // namespace VICUS


#endif // VICUS_BoundaryConditionH
