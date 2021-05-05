#ifndef VICUS_SubSurfaceComponentH
#define VICUS_SubSurfaceComponentH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"
#include "VICUS_Database.h"
#include "VICUS_Window.h"
#include "VICUS_BoundaryCondition.h"

#include <IBK_MultiLanguageString.h>

#include <QString>
#include <QColor>
#include <vector>

namespace VICUS {

class SubSurfaceComponent : public AbstractDBElement {
public:

	/*! SubSurfaceComponent types. */
	enum SubSurfaceComponentType {
		CT_Window,					// Keyword: Window					'A window'
		CT_Door,					// Keyword: Door					'A door'
		CT_Miscellaneous,			// Keyword: Miscellaneous			'Some other component type'
		NUM_CT
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	/*! Checks if all referenced materials exist and if their parameters are valid. */
	bool isValid(const VICUS::Database<VICUS::Window> & windows,
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

	/*! SubSurfaceComponent type. */
	SubSurfaceComponentType			m_type = CT_Miscellaneous;					// XML:E:required

	/*! Transparent construction ID. */
	unsigned int					m_idWindow = INVALID_ID;					// XML:E

	/*! Boundary condition ID for Side A (usually outside). */
	unsigned int					m_idSideABoundaryCondition = INVALID_ID;	// XML:E

	/*! Boundary condition ID for Side B (usually inside). */
	unsigned int					m_idSideBBoundaryCondition = INVALID_ID;	// XML:E
};

} // namespace VICUS


#endif // VICUS_SubSurfaceComponentH
