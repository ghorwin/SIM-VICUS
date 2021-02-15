#include "SVUndoModifyBuilding.h"
#include "SVProjectHandler.h"

SVUndoModifyBuilding::SVUndoModifyBuilding(	const QString & label, const VICUS::Building & b, unsigned int buildingIndex) :
	m_building(b), m_buildingIndex(buildingIndex)
{
	setText( label );
}


void SVUndoModifyBuilding::undo() {
	// exchange building meta data
	std::swap( theProject().m_buildings[m_buildingIndex], m_building);

	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged);
}


void SVUndoModifyBuilding::redo() {
	undo(); // same code as undo
}
