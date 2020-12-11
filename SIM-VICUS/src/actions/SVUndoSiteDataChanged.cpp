#include "SVUndoSiteDataChanged.h"
#include "SVProjectHandler.h"

SVUndoSiteDataChanged::SVUndoSiteDataChanged(	const QString & label,
								double gridWidth, double gridSpacing, double farDistance) :
	m_gridWidth(gridWidth),
	m_gridSpacing(gridSpacing),
	m_farDistance(farDistance)
{
	setText( label );
}


void SVUndoSiteDataChanged::undo() {

	// exchange Project
	std::swap( theProject().m_viewSettings.m_gridWidth, m_gridWidth );
	std::swap( theProject().m_viewSettings.m_gridSpacing, m_gridSpacing );
	std::swap( theProject().m_viewSettings.m_farDistance, m_farDistance );

	// tell project that the grid has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::GridModified);
}


void SVUndoSiteDataChanged::redo() {
	undo();
}
