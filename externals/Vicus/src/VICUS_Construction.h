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
#include "VICUS_Database.h"
#include "VICUS_Material.h"

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

	/*! Checks if all referenced materials exist and if their parameters are valid. */
	bool isValid(const VICUS::Database<VICUS::Material> & materials) const;

	/*! Computes the u-value. */
	bool calculateUValue(double & UValue, const VICUS::Database<Material> & materials, double ri, double re) const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of construction. */
	unsigned int					m_id = INVALID_ID;		// XML:A:required

	/*! The usage type, used as category in tree view. */
	UsageType						m_usageType;			// XML:E

	/*! Display name of construction. */
	IBK::MultiLanguageString		m_displayName;			// XML:A

	/*! False color. */
	QColor							m_color;				// XML:A

	/*! Notes. */
	IBK::MultiLanguageString		m_notes;				// XML:E

	/*! Data source. */
	IBK::MultiLanguageString		m_dataSource;			// XML:E

	/*! The individual material layers. */
	std::vector<MaterialLayer>		m_materialLayers;		// XML:E
};

} // namespace VICUS


#endif // VICUS_ConstructionH
