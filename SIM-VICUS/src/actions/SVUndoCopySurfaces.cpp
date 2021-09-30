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

#include "SVUndoCopySurfaces.h"
#include "SVProjectHandler.h"

#include <VICUS_Project.h>

SVUndoCopySurfaces::SVUndoCopySurfaces(const QString & label, const std::vector<VICUS::Surface> & copiedSurfaces,
									   const std::set<unsigned int> & deselectedSurfaceUniqueIDs,
									   const std::vector<VICUS::ComponentInstance> & compInstances,
									   const std::vector<VICUS::SubSurfaceComponentInstance> & subSurfCompInstances) :
	m_copiedSurfaces(copiedSurfaces),
	m_deselectedSurfaceUniqueIDs(deselectedSurfaceUniqueIDs),
	m_newComponentInstances(compInstances),
	m_newSubSurfaceComponentInstances(subSurfCompInstances)
{
	setText( label );
}


void SVUndoCopySurfaces::undo() {
	// append copied surfaces
	for (const VICUS::Surface &s : m_copiedSurfaces ) {
		const VICUS::Room * r = dynamic_cast<const VICUS::Room *>(s.m_parent);
		if (r == nullptr)
			theProject().m_plainGeometry.pop_back();
		else
			const_cast<VICUS::Room *>(r)->m_surfaces.pop_back();
	}

	// re-select surfaces that were previously selected
	for (VICUS::Building & b : theProject().m_buildings)
		for (VICUS::BuildingLevel & bl : b.m_buildingLevels)
			for (VICUS::Room & r : bl.m_rooms)
				for (VICUS::Surface & s : r.m_surfaces)
					if (m_deselectedSurfaceUniqueIDs.find(s.uniqueID()) != m_deselectedSurfaceUniqueIDs.end())
						s.m_selected = true;

	// remove appended component instances (if any)
	Q_ASSERT(theProject().m_componentInstances.size() >= m_newComponentInstances.size());
	theProject().m_componentInstances.resize(theProject().m_componentInstances.size() - m_newComponentInstances.size());

	// remove appended sub-surface component instances (if any)
	Q_ASSERT(theProject().m_subSurfaceComponentInstances.size() >= m_newSubSurfaceComponentInstances.size());
	theProject().m_subSurfaceComponentInstances.resize(theProject().m_subSurfaceComponentInstances.size() - m_newSubSurfaceComponentInstances.size());
	theProject().updatePointers();

	// tell project that the geometry has changed (i.e. rebuild navigation tree and scene)
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}


void SVUndoCopySurfaces::redo() {
	// remove appended surfaces
	for (const VICUS::Surface &s : m_copiedSurfaces ) {
		const VICUS::Room *r = dynamic_cast<const VICUS::Room *>(s.m_parent);
		if (r == nullptr)
			theProject().m_plainGeometry.push_back(s);
		else
			const_cast<VICUS::Room *>(r)->m_surfaces.push_back(s);
	}

	// deselect surfaces that were previously selected
	for (VICUS::Building & b : theProject().m_buildings)
		for (VICUS::BuildingLevel & bl : b.m_buildingLevels)
			for (VICUS::Room & r : bl.m_rooms)
				for (VICUS::Surface & s : r.m_surfaces)
					if (m_deselectedSurfaceUniqueIDs.find(s.uniqueID()) != m_deselectedSurfaceUniqueIDs.end()) {
						s.m_selected = false;
						for (const VICUS::SubSurface & subS : s.subSurfaces() )
							const_cast<VICUS::SubSurface &>(subS).m_selected = false;
					}


	// append component instances (if vector is empty, nothing happens here)
	theProject().m_componentInstances.insert(theProject().m_componentInstances.end(),
										 m_newComponentInstances.begin(), m_newComponentInstances.end());
	// append component instances (if vector is empty, nothing happens here)
	theProject().m_subSurfaceComponentInstances.insert(theProject().m_subSurfaceComponentInstances.end(),
										 m_newSubSurfaceComponentInstances.begin(), m_newSubSurfaceComponentInstances.end());
	theProject().updatePointers();

	// tell project that the building geometry has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}
