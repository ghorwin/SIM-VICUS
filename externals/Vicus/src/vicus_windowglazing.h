#ifndef VICUS_WindowGlazingH
#define VICUS_WindowGlazingH

#include <string>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"

namespace VICUS {


class WindowGlazing
{
public:
	WindowGlazing();

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID



	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique id number. */
	unsigned int						m_id = INVALID_ID;				// XML:A:required
	/*! Display name of Glazing. */
	std::string							m_displayName;					// XML:A


};

}
#endif // VICUS_WindowGlazingH
