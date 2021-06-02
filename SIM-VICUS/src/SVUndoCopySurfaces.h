/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef SVUndoCopySurfacesH
#define SVUndoCopySurfacesH

#include <VICUS_Surface.h>
#include <VICUS_ComponentInstance.h>

#include "SVUndoCommandBase.h"

/*! Action for copying a surface, either to a room, or as anonymous surface object. */
class SVUndoCopySurfaces : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoCopySurfaces)
public:
	/*! Constructor, allowing different ways for copying a surface:
		1. annonymous surface (actually just a polygon) without associated room or component.
		2. surface belonging to a room
		3. surface belonging to a room, and getting a component instance association.

		\param compInstance If not nullptr, the component instance is being added to the project. No ownership transfer!
	*/
	SVUndoCopySurfaces(const QString & label, const std::vector<VICUS::Surface> & copiedSurfaces,
					   const std::set<unsigned int> & deselectedSurfaceUniqueIDs,
					   const std::vector<VICUS::ComponentInstance> & compInstances,
					   const std::vector<VICUS::SubSurfaceComponentInstance> & subSurfCompInstances);

	virtual void undo();
	virtual void redo();

private:

	/*! Cache for copied surface. */
	std::vector<VICUS::Surface>				m_copiedSurfaces;
	/*! UniqueIDs of original surfaces that need to be deselected. */
	std::set<unsigned int>					m_deselectedSurfaceUniqueIDs;
	/*! Optionally copied component instances. */
	std::vector<VICUS::ComponentInstance>	m_newComponentInstances;
	/*! Optionally copied sub Surface component instances. */
	std::vector<VICUS::SubSurfaceComponentInstance>	m_newSubSurfaceComponentInstances;
};


#endif // SVUndoCopySurfacesH
