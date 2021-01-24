#include "SVUndoModifyComponentInstances.h"
#include "SVProjectHandler.h"

SVUndoModifyComponentInstances::SVUndoModifyComponentInstances(	const QString & label,
								const std::vector<VICUS::ComponentInstance> & ci) :
	m_componentInstances(ci)
{
	setText( label );
}


void SVUndoModifyComponentInstances::undo() {
	// exchange Project data
	std::swap( theProject().m_componentInstances, m_componentInstances);

	// tell project that the grid has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::ComponentInstancesModified);
}


void SVUndoModifyComponentInstances::redo() {
	undo(); // same code as undo
}
