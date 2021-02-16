#include "SVUndoModifyBuildingLevel.h"
#include "SVProjectHandler.h"

SVUndoModifyBuildingLevel::SVUndoModifyBuildingLevel(const QString & label, const VICUS::BuildingLevel & bl, unsigned int buildingIndex, unsigned int buildingLevelIndex, bool withoutRooms) :
	m_buildingLevel(bl), m_buildingIndex(buildingIndex), m_buildingLevelIndex(buildingLevelIndex), m_withoutRooms(withoutRooms)
{
	setText( label );
}


void SVUndoModifyBuildingLevel::undo() {
	// exchange building meta data
	std::swap( theProject().m_buildings[m_buildingIndex].m_buildingLevels[m_buildingLevelIndex], m_buildingLevel);
	if (m_withoutRooms) {
		std::swap( theProject().m_buildings[m_buildingIndex].m_buildingLevels[m_buildingLevelIndex].m_rooms, m_buildingLevel.m_rooms);
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
	}
	else {
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
	}
}


void SVUndoModifyBuildingLevel::redo() {
	undo(); // same code as undo
}
