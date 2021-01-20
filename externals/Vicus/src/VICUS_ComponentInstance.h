#ifndef VICUS_ComponentInstanceH
#define VICUS_ComponentInstanceH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"

namespace VICUS {

class Component;
class Surface;

/*! Defines a construction/component of the building.
	It is basically just a connection data member and does not hold any
	physical parameters itself.
*/
class ComponentInstance {
public:
	/*! Default c'tor. */
	ComponentInstance() {}
	/*! Initializing constructor. */
	ComponentInstance(unsigned int id,
					  unsigned int componentID, unsigned int sideASurfaceID, unsigned int sideBSurfaceID)
		: m_id(id), m_componentID(componentID), m_sideASurfaceID(sideASurfaceID), m_sideBSurfaceID(sideBSurfaceID)
	{}

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	// *** PUBLIC MEMBER VARIABLES ***

	/*! ID of component instance (will be used for NANDRAD::ConstructionInstance). */
	unsigned int						m_id = INVALID_ID;					// XML:A:required
	/*! ID of referenced component. */
	unsigned int						m_componentID = INVALID_ID;			// XML:A
	/*! ID of surface at side A (optional, ID = 0 reserved for "ambient", INVALID_ID means adiabatic). */
	unsigned int						m_sideASurfaceID = INVALID_ID;		// XML:A
	/*! ID of surface at side B (optional, ID = 0 reserved for "ambient", INVALID_ID means adiabatic). */
	unsigned int						m_sideBSurfaceID = INVALID_ID;		// XML:A


	// *** RUNTIME VARIABLES ***

	// These pointers are updated in VICUS::Project::updatePointers() and can be used
	// to quicky travers the data model.

	VICUS::Component	*m_component		= nullptr;
	VICUS::Surface		*m_sideASurface		= nullptr;
	VICUS::Surface		*m_sideBSurface		= nullptr;
};

} // namespace VICUS


#endif // VICUS_ComponentInstanceH
