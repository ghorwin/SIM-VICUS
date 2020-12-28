#include "SVUndoAddSurface.h"
#include "SVProjectHandler.h"

SVUndoAddSurface::SVUndoAddSurface(const QString & label, const VICUS::Surface & addedSurface, unsigned int parentNodeID) :
	m_addedSurface(addedSurface),
	m_parentNodeID(parentNodeID)
{
	setText( label );
}


void SVUndoAddSurface::undo() {

	// remove last building
	Q_ASSERT(!theProject().m_plainGeometry.empty());

	// find room, if given
	if (m_parentNodeID != 0) {

	}
	else {
		theProject().m_plainGeometry.pop_back();
	}

	// tell project that the geometry has changed (i.e. rebuild navigation tree and scene)
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}


void SVUndoAddSurface::redo() {
	// find room, if given
	if (m_parentNodeID != 0) {

//		for (VICUS::Building & b : theProject().m_buildings) {
//			VICUS::Object * obj = b.findChild(m_parentNodeID);
//			if (obj != nullptr) {
//				obj
//			}
//		}
	}
	else {
		// add to anonymous geometry
		theProject().m_plainGeometry.push_back(m_addedSurface);
	}

	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}

