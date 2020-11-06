#ifndef VICUS_RoomH
#define VICUS_RoomH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_Surface.h"

#include <QString>

namespace VICUS {

class Room {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of building. */
	unsigned int					m_id = INVALID_ID;			// XML:A:required

	QString							m_displayName;				// XML:E

	/*! Surfaces of the room. */
	std::vector<Surface>			m_surfaces;					//XML:E
};

} // namespace VICUS


#endif // VICUS_RoomH
