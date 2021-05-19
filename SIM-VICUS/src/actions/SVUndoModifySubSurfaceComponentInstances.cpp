#include "SVUndoModifySubSurfaceComponentInstances.h"
#include "SVProjectHandler.h"

SVUndoModifySubSurfaceComponentInstances::SVUndoModifySubSurfaceComponentInstances(const QString & label,
								const std::vector<VICUS::SubSurfaceComponentInstance> & ci) :
	m_componentInstances(ci)
{
	setText( label );
}


void SVUndoModifySubSurfaceComponentInstances::undo() {
	// exchange Project data
	std::swap( theProject().m_subSurfaceComponentInstances, m_componentInstances);
	theProject().updatePointers();

	// tell project that the grid has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::SubSurfaceComponentInstancesModified);
}


void SVUndoModifySubSurfaceComponentInstances::redo() {
	undo(); // same code as undo
}
