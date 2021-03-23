#ifndef VICUS_InfiltrationH
#define VICUS_InfiltrationH

#include <QColor>

#include <IBK_Parameter.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"

namespace VICUS {

/*! Describes the course of all infiltration.

*/

class Infiltration : public AbstractDBElement {
public:

	/*! Basic parameters. */
	enum para_t {
		/*! Air change rate. */
		P_AirChangeRate,				// Keyword: AirChangeRate			[1/h]		'Air change rate.'
		/*! Shielding coefficient. */
		P_ShieldingCoefficient,			// Keyword: ShiedlindCoefficient	[1/h]		'Shielding coefficient for n50 value.'

		NUM_P
	};

	/*! Air Change Type.*/
	enum AirChangeType {
		AC_normal,				// Keyword: normal							[-]		'normal'
		AC_n50,					// Keyword: n50								[-]		'n50'
		NUM_AC
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	/*! Checks if all parameters are valid. */
	bool isValid() const;


	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of Intenal Load. */
	unsigned int					m_id = INVALID_ID;						// XML:A:required

	/*! Display name of Intenal Load. */
	IBK::MultiLanguageString		m_displayName;							// XML:A

	/*! False color. */
	QColor							m_color;								// XML:A

	/*! Notes. */
	IBK::MultiLanguageString		m_notes;								// XML:E

	/*! Data source. */
	IBK::MultiLanguageString		m_dataSource;							// XML:E

	/*! Air change type. */
	AirChangeType					m_airChangeType = NUM_AC;				// XML:E

	/*! Schedule ID. */
	unsigned int					m_managementScheduleId = INVALID_ID;	// XML:E

	/*! List of parameters. */
	IBK::Parameter					m_para[NUM_P];							// XML:E
};

}
#endif // VICUS_InfiltrationH
