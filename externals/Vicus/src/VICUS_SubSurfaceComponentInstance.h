#ifndef VICUS_SubSurfaceComponentInstanceH
#define VICUS_SubSurfaceComponentInstanceH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"

namespace VICUS {

class SubSurface;

/*! Defines an embedded object (a subsurface).
	It is basically just a connection data member and does not hold any
	physical parameters itself.
*/
class SubSurfaceComponentInstance {
public:
	/*! Default c'tor. */
	SubSurfaceComponentInstance() {}
	/*! Initializing constructor. */
	SubSurfaceComponentInstance(unsigned int id,
					  unsigned int componentID, unsigned int sideASurfaceID, unsigned int sideBSurfaceID)
		: m_id(id), m_subSurfaceComponentID(componentID), m_sideASurfaceID(sideASurfaceID), m_sideBSurfaceID(sideBSurfaceID)
	{}

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	// *** PUBLIC MEMBER VARIABLES ***

	/*! ID of component instance (will be used for NANDRAD::ConstructionInstance). */
	unsigned int						m_id = INVALID_ID;						// XML:A:required
	/*! ID of referenced component. */
	unsigned int						m_subSurfaceComponentID = INVALID_ID;	// XML:A
	/*! ID of surface at side A (optional, ID = 0 reserved for "ambient", INVALID_ID means adiabatic). */
	unsigned int						m_sideASurfaceID = INVALID_ID;			// XML:A
	/*! ID of surface at side B (optional, ID = 0 reserved for "ambient", INVALID_ID means adiabatic). */
	unsigned int						m_sideBSurfaceID = INVALID_ID;			// XML:A


	// *** RUNTIME VARIABLES ***

	// These pointers are updated in VICUS::Project::updatePointers() and can be used
	// to quicky travers the data model.

	VICUS::SubSurface					*m_sideASubSurface		= nullptr;
	VICUS::SubSurface					*m_sideBSubSurface		= nullptr;
};

} // namespace VICUS


#endif // VICUS_SubSurfaceComponentInstanceH
