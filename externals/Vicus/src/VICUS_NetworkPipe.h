#ifndef NETWORKPIPE_H
#define NETWORKPIPE_H

#include <string>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"

namespace VICUS {

class NetworkPipe {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique id number. */
	unsigned int						m_id = INVALID_ID;				// XML:A:required
	/*! Display name of fluid. */
	std::string							m_displayName;					// XML:A

	/*! Outside diameter in [mm]. */
	//ToDo Hauke
	//ist hier nicht besser den innendurchmesser und dann die konstruktion mit deren dicke anzugeben?
	//rohrhersteller schreiben doch auch 20x2
	//zudem könnte man dann gleich eine referenz auf das material setzen
	double								m_dOutside;						// XML:A:required
	/*! Wall thickness in [mm]. */
	double								m_sWall;						// XML:A:required
	/*! roughness in [mm]. */
	double								m_roughness;					// XML:A:required
};

} // namespace VICUS

#endif // NETWORKPIPE_H
