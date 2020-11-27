#ifndef VICUS_BoundaryConditionH
#define VICUS_BoundaryConditionH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"

#include <QString>
#include <vector>

namespace VICUS {

class BoundaryCondition {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of building. */
	unsigned int					m_id = INVALID_ID;		// XML:A:required

	/*! Display name of building. */
	QString							m_displayName;			// XML:A

	/*! Vector of building levels. */
	//std::vector<BuildingLevel>		m_buildingLevels;		// XML:E
};

} // namespace VICUS


#endif // VICUS_BoundaryConditionH
