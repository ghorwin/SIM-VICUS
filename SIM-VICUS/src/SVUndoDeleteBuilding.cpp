#include "SVUndoDeleteBuilding.h"

SVUndoDeleteBuilding::SVUndoDeleteBuilding(const QString & label, unsigned int buildingIndex)
	: m_buildingIndex(buildingIndex)
{
	setText( label );

	Q_ASSERT(project().m_buildings.size() > buildingIndex);

	m_deletedBuilding = project().m_buildings[buildingIndex];
}


void SVUndoDeleteBuilding::undo() {

	theProject().m_buildings.insert(theProject().m_buildings.begin() + m_buildingIndex, m_deletedBuilding);
	theProject().updatePointers();

	// tell project that the building has changed
	// if building object is empty, we only have a topolopy change, otherwise a full geometry update
	if (m_deletedBuilding.m_buildingLevels.empty())
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
	else
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}


void SVUndoDeleteBuilding::redo() {
	Q_ASSERT(!theProject().m_buildings.empty());

	theProject().m_buildings.erase(theProject().m_buildings.begin() + m_buildingIndex);
	theProject().updatePointers();

	// tell project that the building has changed
	// if building object is empty, we only have a topolopy change, otherwise a full geometry update
	if (m_deletedBuilding.m_buildingLevels.empty())
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
	else
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}

