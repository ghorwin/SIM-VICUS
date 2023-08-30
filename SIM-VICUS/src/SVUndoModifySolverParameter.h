#ifndef SVUNDOMODIFYSOLVERPARAMETER_H
#define SVUNDOMODIFYSOLVERPARAMETER_H

#include "SVUndoCommandBase.h"

#include <NANDRAD_SolverParameter.h>
#include <NANDRAD_SimulationParameter.h>


class SVUndoModifySolverParameter : public SVUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(SVUndoModifySolverParams)
public:
	SVUndoModifySolverParameter(const QString & label,
								const NANDRAD::SolverParameter & solverParams, const NANDRAD::SimulationParameter & simulationParams);

	virtual void undo();
	virtual void redo();

private:

	/*! Solver parameters for NANDRAD. */
	NANDRAD::SolverParameter							m_solverParameter;			// XML:E
	/*! Simulation parameters for NANDRAD. */
	NANDRAD::SimulationParameter						m_simulationParameter;		// XML:E

};

#endif // SVUNDOMODIFYSOLVERPARAMETER_H
