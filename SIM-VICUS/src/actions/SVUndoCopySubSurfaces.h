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

#ifndef SVUndoCopySubSurfacesH
#define SVUndoCopySubSurfacesH

#include <VICUS_SubSurface.h>
#include <VICUS_SubSurfaceComponentInstance.h>

#include "SVUndoCommandBase.h"

/*! Action for copying a sub surface, either to a surface.
	Several selected subsurfaces from different parent surfaces can be copied at the same time.
	If these surfaces shall be assigned to a different surface
*/
class SVUndoCopySubSurfaces : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoCopySubSurfaces)
public:
	SVUndoCopySubSurfaces(const QString & label,
						  const std::vector<VICUS::SubSurface> & copiedSubSurfaces,
						  const std::set<unsigned int> & deselectedSurfaceUniqueIDs,
						  const std::vector<VICUS::SubSurfaceComponentInstance> & subSurfCompInstances);

	virtual void undo();
	virtual void redo();

private:

	/*! Cache for copied surface. */
	std::vector<VICUS::SubSurface>						m_copiedSubSurfaces;
	/*! UniqueIDs of original surfaces that need to be deselected. */
	std::set<unsigned int>								m_deselectedSubSurfaceUniqueIDs;
	/*! Optionally copied sub Surface component instances. */
	std::vector<VICUS::SubSurfaceComponentInstance>		m_newSubSurfaceComponentInstances;
};


#endif // SVUndoCopySubSurfacesH
