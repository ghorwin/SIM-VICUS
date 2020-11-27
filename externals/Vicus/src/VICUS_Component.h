#ifndef VICUS_ComponentH
#define VICUS_ComponentH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"

#include <QString>
#include <QColor>
#include <vector>

namespace VICUS {

class Component {
public:

	/*! Component types. */
	enum type {
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
		Num_CK
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of component. */
	unsigned int					m_id = INVALID_ID;		// XML:A:required

	/*! Display name of component. */
	QString							m_displayName;			// XML:A

	/*! False color. */
	QColor							m_color;				// XML:A

	/*! Manufacturer. */
	QString							m_manufacturer;			// XML:E

	/*! Data source. */
	QString							m_dataSource;			// XML:E

	/*! Component type. */
	type							m_type = Num_CK;		// XML:E:required

	/*! Construction ID. */
	unsigned int					m_idConstruction;		// XML:E

	/*! Boundary condition ID. */
	unsigned int					m_idBoundaryCondition;	// XML:E

	/*! Surface property ID. */
	unsigned int					m_idSurfaceProperty;	// XML:E
};

} // namespace VICUS


#endif // VICUS_ComponentH
