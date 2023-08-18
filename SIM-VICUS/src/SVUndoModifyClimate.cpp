#include "SVUndoModifyClimate.h"

#include "SVProjectHandler.h"

#include <VICUS_Project.h>


SVUndoModifyClimate::SVUndoModifyClimate(const QString & label, const NANDRAD::Location & location, bool climateFileModified):
	m_location(location),
	m_climateFileModified(climateFileModified)
{
	setText( label );
}


void SVUndoModifyClimate::undo() {

	// exchange Project
	std::swap( theProject().m_location, m_location);

	// tell project that the climate has changed
	if (m_climateFileModified)
		SVProjectHandler::instance().setModified( SVProjectHandler::ClimateFileModified);
	else
		SVProjectHandler::instance().setModified( SVProjectHandler::ClimateLocationModified);
}


void SVUndoModifyClimate::redo() {
	undo();
}


