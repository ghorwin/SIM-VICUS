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
		CT_OuterWall,
		CT_OuterWallGround,
		CT_InnerWall,
		CT_SlopedRoof,
		CT_FlatRoof,
		CT_ColdRoof,
		CT_WarmRoof,
		CT_FloorToAir,
		CT_FloorToGround,
		CT_Ceiling,
		CT_Window,
		CT_Door,
		CT_Miscellaneous,
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
	CompontType						m_type = NUM_CT;							// XML:E:required

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
