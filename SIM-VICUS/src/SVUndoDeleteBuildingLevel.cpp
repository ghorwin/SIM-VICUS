#include "SVUndoDeleteBuildingLevel.h"

SVUndoDeleteBuildingLevel::SVUndoDeleteBuildingLevel(const QString & label, unsigned int buildingIndex, unsigned int buildingLevelIndex)
	: m_buildingIndex(buildingIndex), m_levelIndex(buildingLevelIndex)
{
	setText( label );

	Q_ASSERT(project().m_buildings.size() > buildingIndex);

	const VICUS::Building & b = project().m_buildings[buildingIndex];
	Q_ASSERT(b.m_buildingLevels.size() > buildingLevelIndex);

	m_deletedLevel = b.m_buildingLevels[buildingLevelIndex];
}


void SVUndoDeleteBuildingLevel::undo() {

	// re-insert level
	theProject().m_buildings[m_buildingIndex].m_buildingLevels.insert(
				theProject().m_buildings[m_buildingIndex].m_buildingLevels.begin() + m_levelIndex, m_deletedLevel);
	theProject().updatePointers();

	// tell project that the building has changed
	// if level object is empty, we only have a topolopy change, otherwise a full geometry update
	if (m_deletedLevel.m_rooms.empty())
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
	else
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}


void SVUndoDeleteBuildingLevel::redo() {
	theProject().m_buildings[m_buildingIndex].m_buildingLevels.erase(
				theProject().m_buildings[m_buildingIndex].m_buildingLevels.begin() + m_levelIndex);
	theProject().updatePointers();

	// tell project that the building has changed
	// if level object is empty, we only have a topolopy change, otherwise a full geometry update
	if (m_deletedLevel.m_rooms.empty())
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
	else
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}

