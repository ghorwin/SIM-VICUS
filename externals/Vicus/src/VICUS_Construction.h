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

	/*! Component categories.*/
	enum category_t{
		//opaque categories
		MC_Coating,					// Keyword: Coating
		MC_Plaster,					// Keyword: Plaster
		MC_Bricks,					// Keyword: Bricks
		MC_NaturalStones,			// Keyword: NaturalStones
		MC_Cementitious,			// Keyword: Cementitious
		MC_Insulations,				// Keyword: Insulations
		MC_BuildingBoards,			// Keyword: BuildingBoards
		MC_Woodbased,				// Keyword: Woodbased
		MC_NaturalMaterials,		// Keyword: NaturalMaterials
		MC_Soils,					// Keyword: Soils
		MC_CladdingSystems,			// Keyword: CladdingSystems
		MC_Foils,					// Keyword: Foils
		MC_Miscellaneous,			// Keyword: Miscellaneous
		//Glass
		MC_GlassPane,				// Keyword: GlasPane
		//Gas
		MC_Gas,						// Keyword: Gas
		Num_MC
	};
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

	/*! Manufacturer. */
	QString							m_manufacturer;			// XML:E

	/*! Data source. */
	QString							m_dataSource;			// XML:E

	IBK::Flag						m_isOpaque;				// XML:A:required

	std::vector<MaterialLayer>		m_materialLayers;		// XML:E

	//Transparent Construction
	/*! Frame ID. */
	unsigned int					m_idFrame;				// XML:E

	/*! Divider ID. */
	unsigned int					m_idDivider;			// XML:E
};

} // namespace VICUS


#endif // VICUS_ConstructionH
