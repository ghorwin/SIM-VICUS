#include "SVUndoAddBuilding.h"
#include "SVProjectHandler.h"

SVUndoAddBuilding::SVUndoAddBuilding(const QString & label, const VICUS::Building & addedBuilding, bool emptyBuildingOnly) :
	m_addedBuilding(addedBuilding),
	m_emptyBuildingOnly(emptyBuildingOnly)
{
	setText( label );
}


void SVUndoAddBuilding::undo() {

	// remove last building
	Q_ASSERT(!theProject().m_buildings.empty());

	theProject().m_buildings.pop_back();

	// tell project that the geometry has changed (i.e. rebuild navigation tree and scene)
	if (m_emptyBuildingOnly)
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
	else
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}


void SVUndoAddBuilding::redo() {
	// append building
	theProject().m_buildings.push_back(m_addedBuilding);
	theProject().updatePointers();

	// tell project that the network has changed
	if (m_emptyBuildingOnly)
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
	else
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}

