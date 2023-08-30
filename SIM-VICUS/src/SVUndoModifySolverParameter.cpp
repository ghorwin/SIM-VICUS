#include "SVUndoModifySolverParameter.h"

#include "SVProjectHandler.h"

#include <VICUS_Project.h>


SVUndoModifySolverParameter::SVUndoModifySolverParameter(const QString & label, const NANDRAD::SolverParameter & solverParams, const NANDRAD::SimulationParameter & simulationParams):
	m_solverParameter(solverParams),
	m_simulationParameter(simulationParams)
{
	setText( label );
}


void SVUndoModifySolverParameter::undo() {

	// exchange Project
	std::swap( theProject().m_solverParameter, m_solverParameter);
	std::swap( theProject().m_simulationParameter, m_simulationParameter);

	// tell project that the grid has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::SolverParametersModified);
}


void SVUndoModifySolverParameter::redo() {
	undo();
}

