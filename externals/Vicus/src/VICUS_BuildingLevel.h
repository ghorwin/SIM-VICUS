#ifndef VICUS_BuildingLevelH
#define VICUS_BuildingLevelH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_Room.h"
#include "VICUS_Object.h"

#include <QString>

namespace VICUS {

class BuildingLevel : public Object {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	void updateParents() {
		m_children.clear();
		for (Room & s : m_rooms) {
			s.m_parent = this;
			m_children.push_back(&s);
			s.updateParents();
		}
	}

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of building level. */
	unsigned int						m_id = INVALID_ID;			// XML:A:required

	QString								m_displayName;				// XML:A

	/*! Stores visibility information for this surface. */
	bool								m_visible = true;			// XML:A

	/*! Vector of all rooms in a building level. */
	std::vector<Room>					m_rooms;					// XML:E
};

} // namespace VICUS


#endif // VICUS_BuildingLevelH
