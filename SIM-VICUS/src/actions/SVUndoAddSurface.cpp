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

#include "SVUndoAddSurface.h"
#include "SVProjectHandler.h"

#include <VICUS_Project.h>

SVUndoAddSurface::SVUndoAddSurface(const QString & label, const VICUS::Surface & addedSurface,
								   unsigned int parentNodeID, const VICUS::ComponentInstance * compInstance) :
	m_addedSurface(addedSurface),
	m_parentNodeID(parentNodeID)
{
	setText( label );
	if (compInstance != nullptr)
		m_componentInstance = *compInstance;
}


void SVUndoAddSurface::undo() {

	// find room, if given
	if (m_parentNodeID != 0) {
		// find the parent room node (it must be a room node!)
		const VICUS::Room * r = dynamic_cast<const VICUS::Room *>(theProject().objectById(m_parentNodeID));
		Q_ASSERT(r != nullptr);
		// remove previously added surface
		Q_ASSERT(!r->m_surfaces.empty());
		const_cast<VICUS::Room*>(r)->m_surfaces.pop_back();
	}
	else {
		Q_ASSERT(!theProject().m_plainGeometry.m_surfaces.empty());
		theProject().m_plainGeometry.m_surfaces.pop_back();
	}
	if (m_componentInstance.m_idComponent != VICUS::INVALID_ID) {
		Q_ASSERT(!theProject().m_componentInstances.empty());
		theProject().m_componentInstances.pop_back();
	}
	theProject().updatePointers();

	// tell project that the geometry has changed (i.e. rebuild navigation tree and scene)
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}


void SVUndoAddSurface::redo() {
	// find room, if given
	if (m_parentNodeID != 0) {

		// find the parent room node (it must be a room node!)
		const VICUS::Room * r = dynamic_cast<const VICUS::Room *>(theProject().objectById(m_parentNodeID));
		Q_ASSERT(r != nullptr);
		// add surface to room surfaces
		const_cast<VICUS::Room*>(r)->m_surfaces.push_back(m_addedSurface);
	}
	else {
		// add to anonymous geometry
		theProject().m_plainGeometry.m_surfaces.push_back(m_addedSurface);
	}

	if (m_componentInstance.m_idComponent != VICUS::INVALID_ID) {
		theProject().m_componentInstances.push_back(m_componentInstance);
	}
	theProject().updatePointers();

	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}

