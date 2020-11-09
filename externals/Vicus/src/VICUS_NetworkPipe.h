#ifndef NETWORKPIPE_H
#define NETWORKPIPE_H

#include <string>

#include "VICUS_CodeGenMacros.h"

namespace VICUS {

class NetworkPipe {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique id number. */
	unsigned int						m_id;							// XML:A:required
	/*! Display name of fluid. */
	std::string							m_displayName;					// XML:A

	/*! Outside diameter in [mm]. */
	double								m_dOutside;						// XML:A:required
	/*! Inside diameter in [mm]. */
	double								m_dInside;						// XML:A:required
};

} // namespace VICUS

#endif // NETWORKPIPE_H
