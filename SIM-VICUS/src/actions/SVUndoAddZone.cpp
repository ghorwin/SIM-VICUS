#include "SVUndoAddZone.h"
#include "SVProjectHandler.h"

SVUndoAddZone::SVUndoAddZone(const QString & label, unsigned int buildingLevelUUID, const VICUS::Room & addedRoom,
							 bool topologyOnly, const std::vector<VICUS::ComponentInstance> * componentInstances) :
	m_addedRoom(addedRoom),
	m_topologyOnly(topologyOnly),
	m_buildingLevelUUID(buildingLevelUUID)
{
	setText( label );
	if (componentInstances != nullptr)
		m_componentInstances = *componentInstances;
}


void SVUndoAddZone::undo() {
	// lookup modified building level
	const VICUS::BuildingLevel * bl = dynamic_cast<const VICUS::BuildingLevel*>(theProject().objectById(m_buildingLevelUUID));
	Q_ASSERT(bl != nullptr);

	// remove last building level
	Q_ASSERT(!bl->m_rooms.empty());

	// remove appended room
	const_cast<VICUS::BuildingLevel *>(bl)->m_rooms.pop_back();

	// remove appended component instances (if any)
	Q_ASSERT(theProject().m_componentInstances.size() >= m_componentInstances.size());
	theProject().m_componentInstances.resize(theProject().m_componentInstances.size() - m_componentInstances.size());

	// tell project that the geometry has changed (i.e. rebuild navigation tree and scene)
	if (m_topologyOnly)
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
	else
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}


void SVUndoAddZone::redo() {
	// lookup modified building level
	const VICUS::BuildingLevel * bl = dynamic_cast<const VICUS::BuildingLevel*>(theProject().objectById(m_buildingLevelUUID));
	Q_ASSERT(bl != nullptr);

	// append building level
	const_cast<VICUS::BuildingLevel *>(bl)->m_rooms.push_back(m_addedRoom);
	// append component instances (if vector is empty, nothing happens here)
	theProject().m_componentInstances.insert(theProject().m_componentInstances.end(), m_componentInstances.begin(), m_componentInstances.end());
	theProject().updatePointers();

	// tell project that the building geometry has changed
	if (m_topologyOnly)
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
	else
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}

