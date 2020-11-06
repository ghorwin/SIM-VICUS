#ifndef VICUS_SurfaceH
#define VICUS_SurfaceH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"

#include <IBKMK_Vector3D.h>

#include <QString>

namespace VICUS {

class Surface {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of building. */
	unsigned int						m_id = INVALID_ID;			// XML:A:required

	QString								m_displayName;				// XML:E

	/*! Points of polyline. */
	std::vector<IBKMK::Vector3D>		m_polyline;
};

} // namespace VICUS


#endif // VICUS_SurfaceH
