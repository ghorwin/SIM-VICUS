#include "SVUndoProject.h"
#include "SVProjectHandler.h"

SVUndoProject::SVUndoProject(	const QString & label,
								const VICUS::Project & newProject) :
	m_project(newProject)
{
	setText( label );
}


void SVUndoProject::undo() {

	// exchange Project
	std::swap( theProject(), m_project );

	// tell project that the grid has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::AllModified);
}


void SVUndoProject::redo() {
	undo();
}
