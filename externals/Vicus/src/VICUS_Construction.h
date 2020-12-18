#ifndef VICUS_ConstructionH
#define VICUS_ConstructionH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_MaterialLayer.h"

#include <QString>
#include <QColor>

#include <vector>

#include <IBK_Flag.h>
#include <IBK_Parameter.h>

#include "VICUS_AbstractDBElement.h"

namespace VICUS {

class Construction : public AbstractDBElement {
public:
	enum UsageType {
		UT_OutsideWall,		// Keyword: OutsideWall		'Outside wall construction'
		UT_InsideWall,		// Keyword: InsideWall		'Interior construction'
		UT_Floor,			// Keyword: FloorWall		'Floor/ceiling construction'
		NUM_UT
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	///TODO Andreas : sollen hier funktionen rein für die Berechnung des U-Werte etc.?

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of construction. */
	unsigned int					m_id = INVALID_ID;		// XML:A:required

	/*! The usage type, used as category in tree view. */
	UsageType						m_usageType;			// XML:E

	/*! Display name of construction. */
	QString							m_displayName;			// XML:A

	/*! False color. */
	QColor							m_color;				// XML:A

	/*! Notes. */
	QString							m_notes;				// XML:E

	/*! Manufacturer. */
	QString							m_manufacturer;			// XML:E

	/*! Data source. */
	QString							m_dataSource;			// XML:E

	std::vector<MaterialLayer>		m_materialLayers;		// XML:E
};

} // namespace VICUS


#endif // VICUS_ConstructionH
