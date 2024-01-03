#include "SVUndoModifyDrawingFile.h"

#include <VICUS_Project.h>

#include "SVProjectHandler.h"

SVUndoModifyDrawingFile::SVUndoModifyDrawingFile(const QString & label, const IBK::Path &drawingFile):
	m_drawingFile(drawingFile)
{
	setText( label );
}


void SVUndoModifyDrawingFile::undo() {

	// exchange drawing file
	std::swap( theProject().m_drawingFilePath, m_drawingFile);

	// remove current drawings and read new ones
	theProject().m_drawings.clear();
	IBK::Path absDrawingFilePath = SVProjectHandler::instance().replacePathPlaceholders(theProject().m_drawingFilePath);
	if (absDrawingFilePath.exists()) {
		theProject().readDrawingXML(absDrawingFilePath);
		theProject().updatePointers();
		// generate inserts, this should happen only once!
		for (VICUS::Drawing &dr: theProject().m_drawings)
			dr.generateInsertGeometries(theProject().nextUnusedID());
	}
	else
		theProject().updatePointers();

	// tell project that the file has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::DrawingModified);
}


void SVUndoModifyDrawingFile::redo() {
	undo();
}
