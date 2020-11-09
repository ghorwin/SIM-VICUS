#ifndef VICUS_BuildingLevelH
#define VICUS_BuildingLevelH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_Room.h"

#include <QString>

namespace VICUS {

class BuildingLevel {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of building level. */
	unsigned int				m_id = INVALID_ID;			// XML:A:required

	QString						m_displayName;				// XML:A

	/*! Vector of all rooms in a builing level. */
	std::vector<Room>			m_rooms;					// XML:E
};

} // namespace VICUS


#endif // VICUS_BuildingLevelH
