/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

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
		: m_id(id), m_idSubSurfaceComponent(componentID), m_idSideASurface(sideASurfaceID), m_idSideBSurface(sideBSurfaceID)
	{}

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	// *** PUBLIC MEMBER VARIABLES ***

	/*! ID of component instance (will be used for NANDRAD::ConstructionInstance). */
	unsigned int						m_id = INVALID_ID;						// XML:A:required
	/*! ID of referenced component. */
	unsigned int						m_idSubSurfaceComponent = INVALID_ID;	// XML:A
	/*! ID of surface at side A (optional, ID = 0 reserved for "ambient", INVALID_ID means adiabatic). */
	unsigned int						m_idSideASurface = INVALID_ID;			// XML:A
	/*! ID of surface at side B (optional, ID = 0 reserved for "ambient", INVALID_ID means adiabatic). */
	unsigned int						m_idSideBSurface = INVALID_ID;			// XML:A


	/*! ID of shading system definition, to be used when shading system is active. */
	unsigned int						m_idShadingSystem				= INVALID_ID;	// XML:E
	/*! ID of shading control model. */
	unsigned int						m_idShadingControlModel			= INVALID_ID;	// XML:E


	// *** RUNTIME VARIABLES ***

	// These pointers are updated in VICUS::Project::updatePointers() and can be used
	// to quicky travers the data model.

	VICUS::SubSurface					*m_sideASubSurface		= nullptr;
	VICUS::SubSurface					*m_sideBSubSurface		= nullptr;
};

} // namespace VICUS


#endif // VICUS_SubSurfaceComponentInstanceH
