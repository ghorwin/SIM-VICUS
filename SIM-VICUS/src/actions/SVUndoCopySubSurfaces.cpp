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

#include "SVUndoCopySubSurfaces.h"
#include "SVProjectHandler.h"

#include <VICUS_Project.h>

SVUndoCopySubSurfaces::SVUndoCopySubSurfaces(const QString & label, const std::vector<VICUS::SubSurface> & copiedSubSurfaces,
									   const std::set<unsigned int> & deselectedSubSurfaceUniqueIDs,
									   const std::vector<VICUS::SubSurfaceComponentInstance> & subSurfCompInstances) :
		m_copiedSubSurfaces(copiedSubSurfaces),
		m_deselectedSubSurfaceUniqueIDs(deselectedSubSurfaceUniqueIDs),
		m_newSubSurfaceComponentInstances(subSurfCompInstances)
{
	setText( label );
}


void SVUndoCopySubSurfaces::undo() {
	// append copied surfaces
	for (const VICUS::SubSurface &ss : m_copiedSubSurfaces ) {
		const VICUS::Surface * s = dynamic_cast<const VICUS::Surface *>(s->m_parent);
		if (s != nullptr) {
			const_cast<std::vector<VICUS::SubSurface> &>(s->subSurfaces()).pop_back();
			std::vector<VICUS::Polygon2D> holes = s->geometry().holes();
			holes.pop_back();
			const_cast<VICUS::PlaneGeometry &>(s->geometry()).setHoles(holes);
		}
	}

	// re-select surfaces that were previously selected
	for (VICUS::Building & b : theProject().m_buildings)
		for (VICUS::BuildingLevel & bl : b.m_buildingLevels)
			for (VICUS::Room & r : bl.m_rooms)
				for (VICUS::Surface & s : r.m_surfaces)
					for (const VICUS::SubSurface & ss : s.subSurfaces())
						if (m_deselectedSubSurfaceUniqueIDs.find(ss.uniqueID()) != m_deselectedSubSurfaceUniqueIDs.end())
							const_cast<VICUS::SubSurface&>(ss).m_selected = true;

	// remove appended sub-surface component instances (if any)
	Q_ASSERT(theProject().m_subSurfaceComponentInstances.size() >= m_newSubSurfaceComponentInstances.size());
	theProject().m_subSurfaceComponentInstances.resize(theProject().m_subSurfaceComponentInstances.size() - m_newSubSurfaceComponentInstances.size());
	theProject().updatePointers();

	// tell project that the geometry has changed (i.e. rebuild navigation tree and scene)
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}


void SVUndoCopySubSurfaces::redo() {
	// remove appended surfaces
	for (const VICUS::SubSurface &ss : m_copiedSubSurfaces ) {
		VICUS::Surface *s = dynamic_cast<VICUS::Surface *>(ss.m_parent);
		if (s != nullptr) {
			const_cast<std::vector<VICUS::SubSurface> &>(s->subSurfaces()).push_back(ss);
			std::vector<VICUS::Polygon2D> holes = s->geometry().holes();
			holes.push_back(ss.m_polygon2D);
			const_cast<VICUS::PlaneGeometry &>(s->geometry()).setHoles(holes);
		}
	}

	// deselect surfaces that were previously selected
	for (VICUS::Building & b : theProject().m_buildings)
		for (VICUS::BuildingLevel & bl : b.m_buildingLevels)
			for (VICUS::Room & r : bl.m_rooms)
				for (VICUS::Surface & s : r.m_surfaces)
					for (const VICUS::SubSurface & ss : s.subSurfaces())
						if (m_deselectedSubSurfaceUniqueIDs.find(ss.uniqueID()) != m_deselectedSubSurfaceUniqueIDs.end())
							const_cast<VICUS::SubSurface&>(ss).m_selected = false;



	// append component instances (if vector is empty, nothing happens here)
	theProject().m_subSurfaceComponentInstances.insert(theProject().m_subSurfaceComponentInstances.end(),
										 m_newSubSurfaceComponentInstances.begin(), m_newSubSurfaceComponentInstances.end());
	theProject().updatePointers();

	// tell project that the building geometry has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}
