#include "SVUndoModifyProject.h"
#include "SVProjectHandler.h"

SVUndoModifyProject::SVUndoModifyProject(	const QString & label,
								const VICUS::Project & newProject) :
	m_project(newProject)
{
	setText( label );
}


void SVUndoModifyProject::undo() {

	// exchange Project
	std::swap( theProject(), m_project );

	// tell project that the grid has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::AllModified);
}


void SVUndoModifyProject::redo() {
	undo();
}
