#ifndef VICUS_BuildingH
#define VICUS_BuildingH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"

#include <QString>

namespace VICUS {

class Building {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of building. */
	unsigned int	m_id = INVALID_ID;			// XML:A:required

	QString			m_displayName;				// XML:E
};

} // namespace VICUS


#endif // VICUS_BuildingH
