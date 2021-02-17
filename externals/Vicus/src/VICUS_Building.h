#ifndef VICUS_BuildingH
#define VICUS_BuildingH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_BuildingLevel.h"
#include "VICUS_Object.h"

#include <QString>

#include <vector>

namespace VICUS {

/*! Represents the building level node in the data hierarchy. */
class Building : public Object {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	void updateParents() {
		m_children.clear();
		for (BuildingLevel & s : m_buildingLevels) {
			s.m_parent = this;
			m_children.push_back(&s);
			s.updateParents();
		}
	}

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Persistant ID of building. */
	unsigned int						m_id = INVALID_ID;			// XML:A:required

	/*! Display name of building. */
	QString								m_displayName;				// XML:A

	/*! Stores visibility information for this surface.
		Note: keep the next line - this will cause the code generator to create serialization code
			  for the inherited m_visible variable.
	*/
	//:inherited	bool				m_visible = true;			// XML:A

	/*! Vector of building levels. */
	std::vector<BuildingLevel>			m_buildingLevels;			// XML:E


	// *** RUNTIME VARIABLE ***

	/*! Whole building net floor area in m2.
		We need a update function and/or user input.
		TODO : how is this used?
	*/
	double								m_netFloorArea = -1;
};

} // namespace VICUS


#endif // VICUS_BuildingH
