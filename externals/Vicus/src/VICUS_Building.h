#ifndef VICUS_BuildingH
#define VICUS_BuildingH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_BuildingLevel.h"
#include "VICUS_Object.h"

#include <QString>

#include <vector>

namespace VICUS {

class Building : public Object {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE

	void updateParents() {
		for (BuildingLevel & s : m_buildingLevels) {
			s.m_parent = this;
			s.updateParents();
		}
	}

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of building. */
	unsigned int					m_id = INVALID_ID;		// XML:A:required

	/*! Display name of building. */
	QString							m_displayName;			// XML:A

	/*! Vector of building levels. */
	std::vector<BuildingLevel>		m_buildingLevels;		// XML:E
};

} // namespace VICUS


#endif // VICUS_BuildingH
