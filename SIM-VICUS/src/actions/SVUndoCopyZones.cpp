#include "SVUndoCopyZones.h"
#include "SVProjectHandler.h"

SVUndoCopyZone::SVUndoCopyZone(const QString & label, unsigned int buildingLevelUUID,
							   const std::vector<VICUS::Room> copiedRooms,
							   const std::vector<VICUS::ComponentInstance> * componentInstances) :
	m_copiedRooms(copiedRooms),
	m_buildingLevelUUID(buildingLevelUUID)
{
	setText( label );
	if (componentInstances != nullptr)
		m_componentInstances = *componentInstances;
}


void SVUndoCopyZone::undo() {
	// lookup modified building level
	const VICUS::BuildingLevel * bl = dynamic_cast<const VICUS::BuildingLevel*>(theProject().objectById(m_buildingLevelUUID));
	Q_ASSERT(bl != nullptr);

	// remove last building level
	Q_ASSERT(!bl->m_rooms.empty());

	// remove appended room
	for ( VICUS::Room r : m_copiedRooms )
		const_cast<VICUS::BuildingLevel *>(bl)->m_rooms.pop_back();

	// remove appended component instances (if any)
	Q_ASSERT(theProject().m_componentInstances.size() >= m_componentInstances.size());
	theProject().m_componentInstances.resize(theProject().m_componentInstances.size() - m_componentInstances.size());

	// tell project that the geometry has changed (i.e. rebuild navigation tree and scene)
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
}


void SVUndoCopyZone::redo() {
	// lookup modified building level
	const VICUS::BuildingLevel * bl = dynamic_cast<const VICUS::BuildingLevel*>(theProject().objectById(m_buildingLevelUUID));
	Q_ASSERT(bl != nullptr);

	// append building level
	const_cast<VICUS::BuildingLevel *>(bl)->m_rooms.insert(bl->m_rooms.end(), m_copiedRooms.begin(), m_copiedRooms.end() );
	// append component instances (if vector is empty, nothing happens here)
	theProject().m_componentInstances.insert(theProject().m_componentInstances.end(), m_componentInstances.begin(), m_componentInstances.end());
	theProject().updatePointers();

	// tell project that the building geometry has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
}

