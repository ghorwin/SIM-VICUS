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

SVUndoCopyZones::SVUndoCopyZones(const QString & label, unsigned int buildingLevelUUID,
							   const std::vector<VICUS::Room> copiedRooms,
							   const std::vector<VICUS::ComponentInstance> * componentInstances) :
	m_copiedRooms(copiedRooms),
	m_buildingLevelUUID(buildingLevelUUID)
{
	setText( label );
	if (componentInstances != nullptr)
		m_componentInstances = *componentInstances;
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

	// remove appended component instances (if any)
//	Q_ASSERT(theProject().m_componentInstances.size() >= m_componentInstances.size());
//	theProject().m_componentInstances.resize(theProject().m_componentInstances.size() - m_componentInstances.size());

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

	// append component instances (if vector is empty, nothing happens here)
//	theProject().m_componentInstances.insert(theProject().m_componentInstances.end(), m_componentInstances.begin(), m_componentInstances.end());
	theProject().updatePointers();

	// tell project that the building geometry has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
}

