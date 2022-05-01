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

#include "SVUndoCopyZones.h"
#include "SVProjectHandler.h"

#include <VICUS_Project.h>

SVUndoCopyZones::SVUndoCopyZones(const QString & label,
								const std::vector<VICUS::Room> & copiedRooms,
								const std::set<unsigned int> & deselectedRoomUniqueIDs,
								const std::vector<VICUS::ComponentInstance> & componentInstances,
								const std::vector<VICUS::SubSurfaceComponentInstance> & subSurfacecomponentInstances) :
	m_copiedRooms(copiedRooms),
	m_deselectedUniqueIDs(deselectedRoomUniqueIDs),
	m_newComponentInstances(componentInstances),
	m_newSubSurfaceComponentInstances(subSurfacecomponentInstances)
{
	setText( label );
}


void SVUndoCopyZones::undo() {

	// remove appended room
	for ( VICUS::Room r : m_copiedRooms ) {
		const VICUS::BuildingLevel * bl = dynamic_cast<const VICUS::BuildingLevel*>(r.m_parent);
		// lookup modified building level
		Q_ASSERT(bl != nullptr);

		// remove last building level
		Q_ASSERT(!bl->m_rooms.empty());
		const_cast<VICUS::BuildingLevel *>(bl)->m_rooms.pop_back();
	}

	// re-select surfaces that were previously selected
	for (VICUS::Building & b : theProject().m_buildings)
		for (VICUS::BuildingLevel & bl : b.m_buildingLevels)
			for (VICUS::Room & r : bl.m_rooms) {
				if (m_deselectedUniqueIDs.find(r.m_id) != m_deselectedUniqueIDs.end())
					r.m_selected = true;
				for (VICUS::Surface & s : r.m_surfaces) {
						if (m_deselectedUniqueIDs.find(s.m_id) != m_deselectedUniqueIDs.end())
							s.m_selected = true;
				}
			}

	// remove appended component instances (if any)
	Q_ASSERT(theProject().m_componentInstances.size() >= m_newComponentInstances.size());
	theProject().m_componentInstances.resize(theProject().m_componentInstances.size() - m_newComponentInstances.size());

	// remove appended sub-surface component instances (if any)
	Q_ASSERT(theProject().m_subSurfaceComponentInstances.size() >= m_newSubSurfaceComponentInstances.size());
	theProject().m_subSurfaceComponentInstances.resize(theProject().m_subSurfaceComponentInstances.size() - m_newSubSurfaceComponentInstances.size());

	// update pointers
	theProject().updatePointers();

	// tell project that the geometry has changed (i.e. rebuild navigation tree and scene)
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
}


void SVUndoCopyZones::redo() {

	// append building level
	// remove appended room
	for ( VICUS::Room r : m_copiedRooms ) {
		const VICUS::BuildingLevel * bl = dynamic_cast<const VICUS::BuildingLevel*>(r.m_parent);
		// lookup modified building level
		Q_ASSERT(bl != nullptr);

		// remove last building level
		Q_ASSERT(!bl->m_rooms.empty());

		const_cast<VICUS::BuildingLevel *>(bl)->m_rooms.push_back(r);
	}

	// de-select surfaces that were previously selected
	for (VICUS::Building & b : theProject().m_buildings)
		for (VICUS::BuildingLevel & bl : b.m_buildingLevels)
			for (VICUS::Room & r : bl.m_rooms) {
				if (m_deselectedUniqueIDs.find(r.m_id) != m_deselectedUniqueIDs.end())
					r.m_selected = false;
				for (VICUS::Surface & s : r.m_surfaces) {
						if (m_deselectedUniqueIDs.find(s.m_id) != m_deselectedUniqueIDs.end())
							s.m_selected = false;
						for (const VICUS::SubSurface & subS : s.subSurfaces() )
							const_cast<VICUS::SubSurface &>(subS).m_selected = false;
				}
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
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
}

