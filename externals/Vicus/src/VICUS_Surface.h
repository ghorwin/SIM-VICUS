#ifndef VICUS_SurfaceH
#define VICUS_SurfaceH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_PlaneGeometry.h"

#include <QString>

namespace VICUS {

/*! Represents a surface and its associated properties. */
class Surface {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of building. */
	unsigned int						m_id = INVALID_ID;			// XML:A:required

	QString								m_displayName;				// XML:A

	/*! The actual geometry. */
	PlaneGeometry						m_geometry;					// XML:E
};

} // namespace VICUS


#endif // VICUS_SurfaceH
