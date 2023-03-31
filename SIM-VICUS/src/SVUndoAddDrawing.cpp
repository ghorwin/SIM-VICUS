#include "SVUndoAddDrawing.h"
#include "SVProjectHandler.h"
#include "SVSettings.h"

SVUndoAddDrawing::SVUndoAddDrawing(const QString & label, const VICUS::Drawing & addedDrawing) :
	m_addedDrawing(addedDrawing)
{
	setText( label );
}


void SVUndoAddDrawing::undo() {
	// remove drawing
	Q_ASSERT(!theProject().m_drawings.empty());

	theProject().m_drawings.pop_back();

	SVProjectHandler::instance().setModified( SVProjectHandler::DrawingModified);
}


void SVUndoAddDrawing::redo() {
	// append drawing
	theProject().m_drawings.push_back(m_addedDrawing);

	SVProjectHandler::instance().setModified( SVProjectHandler::DrawingModified);
}

