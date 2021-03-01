#ifndef VICUS_InternalLoadH
#define VICUS_InternalLoadH

#include <QColor>

#include <IBK_Parameter.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"

namespace VICUS {

/*! Describes the course of all Internal Loads (Person, Lighting, Equipment, Other).

*/

class InternalLoad : public AbstractDBElement {
public:

	/*! Basic parameters. */
	enum para_t {
		//thermal paramters
		/*! Person Count. */
		P_PersonCount,					// Keyword: Density					[-]		'Person Count.'
		/*! Convective Heat Factor. */
		P_ConvectiveHeatFactor,			// Keyword: HeatCapacity			[---]	'Convective Heat Factor.'

		NUM_P
	};

	/*! Internal load categories.*/
	enum Category {
		IC_Person,				// Keyword: Person							[-]		'Person'
		IC_ElectricEquiment,	// Keyword: ElectricEquiment				[-]		'ElectricEquiment'
		IC_Lighting,			// Keyword: Lighting						[-]		'Lighting'
		IC_Other,				// Keyword: Other							[-]		'Other'
		NUM_MC
	};

	/*! The description is used to identify the unit in the gui. */
	enum PersonCountMethod{
		PCM_PersonPerArea,		// Keyword: PersonPerArea					[-]		'Person per m2'
		PCM_AreaPerPerson,		// Keyword: AreaPerPerson					[-]		'm2 per Person'
		PCM_PersonCount,		// Keyword: PersonCount						[-]		'Person count'
		NUM_PCM
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

	/*! Intenal Load category. */
	Category						m_category = NUM_MC;					// XML:E:required

	/*! Person count method*/
	PersonCountMethod				m_personCountMethod=NUM_PCM;			// XML:E

	unsigned int					m_occupancyScheduleId = INVALID_ID;		// XML:E:required

	/*! only required for person*/
	unsigned int					m_activityScheduleId = INVALID_ID;		// XML:E:

	/*! List of parameters. */
	IBK::Parameter					m_para[NUM_P];							// XML:E
};

}
#endif // VICUS_InternalLoadH
