#ifndef VICUS_RoomH
#define VICUS_RoomH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_Surface.h"
#include "VICUS_Object.h"

#include <QString>

namespace VICUS {

class Room : public Object {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE

	void updateParents() {
		m_children.clear();
		for (Surface & s : m_surfaces) {
			m_children.push_back(&s);
			s.m_parent = this;
		}
	}

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of building. */
	unsigned int						m_id = INVALID_ID;			// XML:A:required

	QString								m_displayName;				// XML:A

	/*! Stores visibility information for this surface. */
	bool								m_visible = true;			// XML:A
	/*! Stores selected information for this surface (not serialized, for now). */
	bool								m_selected = false;

	/*! Surfaces of the room. */
	std::vector<Surface>				m_surfaces;					// XML:E
};

} // namespace VICUS


#endif // VICUS_RoomH
