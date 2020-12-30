#include "SVUndoAddBuildingLevel.h"
#include "SVProjectHandler.h"

SVUndoAddBuildingLevel::SVUndoAddBuildingLevel(const QString & label, unsigned int buildingUUID, const VICUS::BuildingLevel & addedLevel, bool topologyOnly) :
	m_addedLevel(addedLevel),
	m_topologyOnly(topologyOnly),
	m_buildingUUID(buildingUUID)
{
	setText( label );
}


void SVUndoAddBuildingLevel::undo() {
	// lookup modified building
	const VICUS::Building * b = dynamic_cast<const VICUS::Building*>(theProject().objectById(m_buildingUUID));
	Q_ASSERT(b != nullptr);

	// remove last building level
	Q_ASSERT(!b->m_buildingLevels.empty());

	const_cast<VICUS::Building *>(b)->m_buildingLevels.pop_back();

	// tell project that the geometry has changed (i.e. rebuild navigation tree and scene)
	if (m_topologyOnly)
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
	else
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}


void SVUndoAddBuildingLevel::redo() {
	// lookup modified building
	const VICUS::Building * b = dynamic_cast<const VICUS::Building*>(theProject().objectById(m_buildingUUID));
	Q_ASSERT(b != nullptr);

	// append building level
	const_cast<VICUS::Building *>(b)->m_buildingLevels.push_back(m_addedLevel);
	theProject().updatePointers();

	// tell project that the building geometry has changed
	if (m_topologyOnly)
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
	else
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}

