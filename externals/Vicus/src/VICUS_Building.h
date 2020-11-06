#ifndef VICUS_BuildingH
#define VICUS_BuildingH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_BuildingLevel.h"

#include <QString>

#include <vector>

namespace VICUS {

class Building {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of building. */
	unsigned int					m_id = INVALID_ID;		// XML:A:required

	/*! Display name of building. */
	QString							m_displayName;			// XML:E

	/*! Vector of building levels. */
	std::vector<BuildingLevel>		m_buildingLevels;		// XML:E
};

} // namespace VICUS


#endif // VICUS_BuildingH
