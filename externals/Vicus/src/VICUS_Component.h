#ifndef VICUS_ComponentH
#define VICUS_ComponentH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"

#include <IBK_MultiLanguageString.h>

#include <QString>
#include <QColor>
#include <vector>

namespace VICUS {

class Component		: public AbstractDBElement {
public:

	/*! Component types. */
	enum CompontType {
		CT_OutsideWall,				// Keyword: OutsideWall				'Outside wall construction'
		CT_OutsideWallToGround,		// Keyword: OutsideWallToGround		'Outside wall construction in contact with ground'
		CT_InsideWall,				// Keyword: InsideWall				'Interior construction'
		CT_FloorToCellar,			// Keyword: FloorToCellar			'Floor to basement'
		CT_FloorToAir,				// Keyword: FloorToAir				'Floor in contact with air'
		CT_FloorToGround,			// Keyword: FloorToGround			'Floor in contact with ground'
		CT_Ceiling,					// Keyword: Ceiling					'Ceiling construction'
		CT_SlopedRoof,				// Keyword: SlopedRoof				'Sloped roof construction'
		CT_FlatRoof,				// Keyword: FlatRoof				'Flat roof construction'
		CT_ColdRoof,				// Keyword: ColdRoof				'Flat roof construction (to heated/insulated space)'
		CT_WarmRoof,				// Keyword: WarmRoof				'Flat roof construction (to cold/ventilated space)'
//		CT_Window,
//		CT_Door,
		CT_Miscellaneous,			// Keyword: Miscellaneous			'Some other component type'
		NUM_CT
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

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
	CompontType						m_type = CT_Miscellaneous;					// XML:E:required

	/*! Opaque construction ID. */
	unsigned int					m_idOpaqueConstruction = INVALID_ID;		// XML:E

	/*! Transparent construction ID. */
	unsigned int					m_idGlazingSystem = INVALID_ID;				// XML:E

	/*! Outside boundary condition ID. */
	unsigned int					m_idOutsideBoundaryCondition = INVALID_ID;	// XML:E

	/*! Inside boundary condition ID. */
	unsigned int					m_idInsideBoundaryCondition = INVALID_ID;	// XML:E

	/*! Surface property ID. */
	unsigned int					m_idSurfaceProperty = INVALID_ID;			// XML:E
};

} // namespace VICUS


#endif // VICUS_ComponentH
