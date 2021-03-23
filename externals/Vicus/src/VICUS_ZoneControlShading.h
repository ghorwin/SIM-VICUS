#ifndef VICUS_ZoneControlShadingH
#define VICUS_ZoneControlShadingH

#include <QColor>

#include <IBK_Parameter.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"

namespace VICUS {

/*! Describes the course of all zone control shading.

*/

class ZoneControlShading : public AbstractDBElement {
public:

	/*! Basic parameters. */
	enum para_t {
		/*! Global horizontal (upper) sensor value. */
		P_GlobalHorizontal,				// Keyword: GlobalHorizontal		[W/m2]		'Global horizontal (upper) sensor setpoint value.'
		/*! Global north (upper) sensor value. */
		P_GlobalNorth,					// Keyword: GlobalNorth				[W/m2]		'Global north (upper) sensor setpoint value.'
		/*! Global east (upper) sensor value. */
		P_GlobalEast,					// Keyword: GlobalEast				[W/m2]		'Global east (upper) sensor setpoint value.'
		/*! Global south (upper) sensor value. */
		P_GlobalSouth,					// Keyword: GlobalSouth				[W/m2]		'Global south (upper) sensor setpoint value.'
		/*! Global west (upper) sensor value. */
		P_GlobalWest,					// Keyword: GlobalWest				[W/m2]		'Global west (upper) sensor setpoint value.'
		/*! Dead band value. */
		P_DeadBand,						// Keyword: DeadBand				[W/m2]		'Dead band value for all sensors.'


		NUM_P
	};

	/*! Categories.*/
	enum Category {
		C_GlobalHorizontalSensor,				// Keyword: GlobalHorizontalSensor					[-]		'One global horizontal sensor.'
		C_GlobalHorizontalAndVerticalSensors,	// Keyword: GlobalHorizontalAndVerticalSensors		[-]		'One global horizontal and for each direction (N, E, S, W) a vertical sensor.'

		NUM_C
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	/*! Checks if all parameters are valid. */
	bool isValid() const;


	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of zone control shading. */
	unsigned int					m_id = INVALID_ID;						// XML:A:required

	/*! Display name of zone control shading. */
	IBK::MultiLanguageString		m_displayName;							// XML:A

	/*! False color. */
	QColor							m_color;								// XML:A

	/*! Notes. */
	IBK::MultiLanguageString		m_notes;								// XML:E

	/*! Data source. */
	IBK::MultiLanguageString		m_dataSource;							// XML:E

	/*! Category. */
	Category						m_category = NUM_C;						// XML:E:required

	/*! List of parameters. */
	IBK::Parameter					m_para[NUM_P];							// XML:E
};

}
#endif // VICUS_ZoneControlShadingH
