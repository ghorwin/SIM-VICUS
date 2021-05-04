#ifndef VICUS_ComponentH
#define VICUS_ComponentH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"
#include "VICUS_Database.h"
#include "VICUS_Material.h"
#include "VICUS_Construction.h"
#include "VICUS_BoundaryCondition.h"

#include <IBK_MultiLanguageString.h>

#include <QString>
#include <QColor>
#include <vector>

namespace VICUS {

class Component : public AbstractDBElement {
public:

	/*! Component types. */
	enum ComponentType {
		CT_OutsideWall,				// Keyword: OutsideWall				'Outside wall construction'
		CT_OutsideWallToGround,		// Keyword: OutsideWallToGround		'Outside wall construction in contact with ground'
		CT_InsideWall,				// Keyword: InsideWall				'Interior construction'
		CT_FloorToCellar,			// Keyword: FloorToCellar			'Floor to basement'
		CT_FloorToAir,				// Keyword: FloorToAir				'Floor in contact with air'
		CT_FloorToGround,			// Keyword: FloorToGround			'Floor in contact with ground'
		CT_Ceiling,					// Keyword: Ceiling					'Ceiling construction'
		CT_SlopedRoof,				// Keyword: SlopedRoof				'Sloped roof construction'
		CT_FlatRoof,				// Keyword: FlatRoof				'Flat roof construction'
		///TODO Heiko ist der Kommentar hier richtig?
		CT_ColdRoof,				// Keyword: ColdRoof				'Flat roof construction (to heated/insulated space)'
		CT_WarmRoof,				// Keyword: WarmRoof				'Flat roof construction (to cold/ventilated space)'
		CT_Miscellaneous,			// Keyword: Miscellaneous			'Some other component type'
		NUM_CT
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	/*! Checks if all referenced materials exist and if their parameters are valid. */
	bool isValid(const VICUS::Database<VICUS::Material> & materials,
				 const VICUS::Database<VICUS::Construction> & constructions,
				 const VICUS::Database<VICUS::BoundaryCondition> & bcs) const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of component. */
	unsigned int					m_id = INVALID_ID;							// XML:A:required

	/*! Display name of component. */
	IBK::MultiLanguageString		m_displayName;								// XML:A

	/*! False color. */
	QColor							m_color;									// XML:A

	/*! Notes. */
	IBK::MultiLanguageString		m_notes;									// XML:E

	/*! Manufacturer. */
	IBK::MultiLanguageString		m_manufacturer;								// XML:E

	/*! Data source. */
	IBK::MultiLanguageString		m_dataSource;								// XML:E

	/*! Component type. */
	ComponentType					m_type = CT_Miscellaneous;					// XML:E:required

	/*! Opaque construction ID. */
	unsigned int					m_idConstruction = INVALID_ID;				// XML:E

	/*! Boundary condition ID for Side A (usually outside). */
	unsigned int					m_idSideABoundaryCondition = INVALID_ID;	// XML:E

	/*! Boundary condition ID for Side B (usually inside). */
	unsigned int					m_idSideBBoundaryCondition = INVALID_ID;	// XML:E

	/*! Surface property ID.
		TODO Dirk, kann das weg?
	*/
	unsigned int					m_idSurfaceProperty = INVALID_ID;			// XML:E
};

} // namespace VICUS


#endif // VICUS_ComponentH
