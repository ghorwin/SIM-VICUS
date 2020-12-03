#ifndef VICUS_NetworkPipeH
#define VICUS_NetworkPipeH

#include <string>
#include <vector>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"

namespace VICUS {

class NetworkPipe {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	double m_diameterInside() const{
		return m_diameterOutside - 2 * m_sWall;
	}

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique id number. */
	unsigned int						m_id = INVALID_ID;				// XML:A:required
	/*! Display name of fluid. */
	std::string							m_displayName;					// XML:A
	/*! Outside diameter in [mm]. */
	double								m_diameterOutside;				// XML:A:required
	/*! Wall thickness in [mm]. */
	double								m_sWall;						// XML:A:required
	/*! pipe wall roughness in [mm] */
	double								m_roughness;					// XML:A:required

};


} // namespace VICUS

#endif // VICUS_NetworkPipeH
