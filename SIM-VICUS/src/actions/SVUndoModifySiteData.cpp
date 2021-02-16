#include "SVUndoModifySiteData.h"
#include "SVProjectHandler.h"

SVUndoModifySiteData::SVUndoModifySiteData(	const QString & label,
								double gridWidth, double gridSpacing, double farDistance) :
	m_gridWidth(gridWidth),
	m_gridSpacing(gridSpacing),
	m_farDistance(farDistance)
{
	setText( label );
}


void SVUndoModifySiteData::undo() {

	// exchange Project
	std::swap( theProject().m_viewSettings.m_gridWidth, m_gridWidth );
	std::swap( theProject().m_viewSettings.m_gridSpacing, m_gridSpacing );
	std::swap( theProject().m_viewSettings.m_farDistance, m_farDistance );

	// tell project that the grid has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::GridModified);
}


void SVUndoModifySiteData::redo() {
	undo();
}
