#include "SVUndoModifyBuildingTopology.h"
#include "SVProjectHandler.h"

SVUndoModifyBuildingTopology::SVUndoModifyBuildingTopology(const QString & label, const std::vector<VICUS::Building> & buildings) :
	m_buildings(buildings)
{
	setText( label );
}


void SVUndoModifyBuildingTopology::undo() {
	// exchange building meta data
	std::swap( theProject().m_buildings, m_buildings);
	theProject().updatePointers();
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}


void SVUndoModifyBuildingTopology::redo() {
	undo(); // same code as undo
}
