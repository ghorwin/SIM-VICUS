#include "SVUndoAddBuilding.h"
#include "SVProjectHandler.h"

SVUndoAddBuilding::SVUndoAddBuilding(const QString & label, const VICUS::Building & addedBuilding) :
	m_addedBuilding(addedBuilding)
{
	setText( label );
}


void SVUndoAddBuilding::undo() {

	// remove last building
	Q_ASSERT(!theProject().m_buildings.empty());

	theProject().m_buildings.pop_back();

	// tell project that the geometry has changed (i.e. rebuild navigation tree and scene)
	SVProjectHandler::instance().setModified( SVProjectHandler::GeometryChanged);
}


void SVUndoAddBuilding::redo() {
	// append building
	theProject().m_buildings.push_back(m_addedBuilding);
	theProject().updatePointers();

	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::GeometryChanged);
}

