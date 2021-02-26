#ifndef VICUS_InternalLoadH
#define VICUS_InternalLoadH

#include <QColor>

#include <IBK_Parameter.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"


///TODO Katja
/// alle Beschreibungen verbessern
/// Einheiten Info Keyword ...

namespace VICUS {

/*! Describes the course of a *single* scheduled quantity (basically a value over time data set).
 *  This schedule does not have a unit.
*/

class InternalLoad : public AbstractDBElement {
public:

	/*! Basic parameters. */
	enum para_t {
		//thermal paramters
		/*! Dry density of the material. */
		P_PersonCount,					// Keyword: Density					[-]		'Dry density of the material.'
		/*! Specific heat capacity of the material. */
		P_ConvectiveHeatFactor,			// Keyword: HeatCapacity			[---]	'Specific heat capacity of the material.'

		NUM_P
	};

	/*! Internal load categories.*/
	enum Category {
		IC_Person,				// Keyword: Miscellaneous
		IC_ElectricEquiment,	// Keyword: Miscellaneousa
		IC_Lighting,			// Keyword: Miscellaneousaa
		IC_Other,				// Keyword: Miscellaneousaaa
		NUM_MC
	};

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

	/*! Unique ID of material. */
	unsigned int					m_id = INVALID_ID;						// XML:A:required

	/*! Display name of material. */
	IBK::MultiLanguageString		m_displayName;							// XML:A

	/*! False color. */
	QColor							m_color;								// XML:A

	/*! Notes. */
	IBK::MultiLanguageString		m_notes;								// XML:E

	/*! Data source. */
	IBK::MultiLanguageString		m_dataSource;							// XML:E

	/*! Material category. */
	Category						m_category = NUM_MC;					// XML:E:required

	/*! Person count method*/
	PersonCountMethod				m_personCountMethod=NUM_PCM;			// XML:E

	unsigned int					m_occupancyScheduleId = INVALID_ID;		// XML:E:required

	/*! nur für personen erforderlich*/
	unsigned int					m_activityScheduleId = INVALID_ID;		// XML:E:

	/*! List of parameters. */
	IBK::Parameter					m_para[NUM_P];							// XML:E
};

}
#endif // VICUS_InternalLoadH
