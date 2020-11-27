#ifndef VICUS_SurfacePropertiesH
#define VICUS_SurfacePropertiesH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"

#include <QString>
#include <QColor>
#include <vector>

#include <IBK_Parameter.h>

namespace VICUS {

class SurfaceProperties {
public:

	/*! Basic parameters. */
	enum para_t {
		P_Specularity,				// Keyword: Specularity				[---]	'Specularity of the material.'
		P_Roughness,				// Keyword: Roughness				[---]	'Roughness of the material.'
		NUM_P
	};

	enum Type {
		T_Plastic,					// Keyword: Plastic
		T_Metal,					// Keyword: Metal
		T_Glass,					// Keyword: Glass
		NUM_T
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID-number for this glazing system (INVALID_ID = disabled/undefined). */
	unsigned int						m_id = INVALID_ID;							// XML:A:required

	/*! Some display/comment name for this model (optional). */
	QString								m_displayName;								// XML:A

	/*! False color. */
	QColor								m_color;									// XML:A

	/*! Manufacturer. */
	QString								m_manufacturer;								// XML:E

	/*! Data source. */
	QString								m_dataSource;								// XML:E

	/*! List of parameters. */
	IBK::Parameter						m_para[NUM_P];								// XML:E

	Type								m_type = T_Plastic;							// XML:E

};

} // namespace VICUS


#endif // VICUS_SurfacePropertiesH
