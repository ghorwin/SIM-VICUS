#include "SVUndoModifyBuilding.h"
#include "SVProjectHandler.h"

SVUndoModifyBuilding::SVUndoModifyBuilding(const QString & label, const VICUS::Building & b, unsigned int buildingIndex, bool withoutLevels) :
	m_building(b), m_buildingIndex(buildingIndex), m_withoutLevels(withoutLevels)
{
	setText( label );
}


void SVUndoModifyBuilding::undo() {
	// exchange building meta data
	std::swap( theProject().m_buildings[m_buildingIndex], m_building);
	if (m_withoutLevels) {
		std::swap( theProject().m_buildings[m_buildingIndex].m_buildingLevels, m_building.m_buildingLevels);
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
	}
	else {
		SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
	}
}


void SVUndoModifyBuilding::redo() {
	undo(); // same code as undo
}
