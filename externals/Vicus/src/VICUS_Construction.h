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

namespace VICUS {

class Construction {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	///TODO Andreas : sollen hier funktionen rein für die Berechnung des U-Werte etc.?

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of construction. */
	unsigned int					m_id = INVALID_ID;		// XML:A:required

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

	IBK::Flag						m_isOpaque;				// XML:E:required

	std::vector<MaterialLayer>		m_materialLayers;		// XML:E

	//Transparent Construction
	/*! Frame ID. */
	unsigned int					m_idFrame;				// XML:E

	/*! Divider ID. */
	unsigned int					m_idDivider;			// XML:E
};

} // namespace VICUS


#endif // VICUS_ConstructionH
