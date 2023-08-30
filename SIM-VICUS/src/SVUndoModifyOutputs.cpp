#include "SVUndoModifyOutputs.h"

#include "SVProjectHandler.h"

#include <VICUS_Project.h>


SVUndoModifyOutputs::SVUndoModifyOutputs(const QString & label, const VICUS::Outputs &outputs):
	m_outputs(outputs)
{
	setText( label );
}


void SVUndoModifyOutputs::undo() {
	// exchange outputs
	std::swap( theProject().m_outputs, m_outputs);
	SVProjectHandler::instance().setModified( SVProjectHandler::OutputsModified);
}


void SVUndoModifyOutputs::redo() {
	undo();
}

