#ifndef VICUS_RoomH
#define VICUS_RoomH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_Surface.h"
#include "VICUS_Object.h"

#include <QString>

#include <IBK_Parameter.h>

namespace VICUS {

class Room : public Object {
public:

	/*! Room parameters. */
	enum para_t{
		/*! Dry density of the material. */
		P_Area,					// Keyword: Area					[m2]	'Floor area of the zone.'
		/*! Dry density of the material. */
		P_Volume,				// Keyword: Volume					[m3]	'Volume of the zone.'
		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

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

	/*! Stores zone parameters.
		if area or volume is zero --> autocalulation from geometry
	*/
	IBK::Parameter						m_para[NUM_P];				// XML:E

	/*! Surfaces of the room. */
	std::vector<Surface>				m_surfaces;					// XML:E
};

} // namespace VICUS


#endif // VICUS_RoomH
