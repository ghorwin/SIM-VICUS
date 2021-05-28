/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef NM_SteadyStateSolverH
#define NM_SteadyStateSolverH

#include <SOLFRA_ModelInterface.h>

#include <vector>
#include <nvector/nvector_serial.h>

namespace SOLFRA {
	class JacobianInterface;
}

namespace IBKMK {
	class SparseMatrixPattern;
}



namespace NANDRAD_MODEL {

/*!	\brief Declaration for class SteadyStateSolver
	\author Anne Paepcke <anne.paepcke -[at]- tu-dresden.de>

	This class wraps Kinsol solver combined with direct dense
	or sparse solver (dense LU factorized gauss and KLU).
	Steady state calculation are performed from any initial state y0
	at an arbitrary time point of global simulation.
	Inherit from this class and fill the abstract functions
	setY, ydot, setTime and y0 fro SOLFRA:::ModelInterface.

	\code
	// Initial condition vector, size n.
	virtual const double * y0() const {return &m_states[0];}

	// We ignore setTime.
	virtual SOLFRA::ModelInterface::CalculationResult setTime(double t)
	{ return SOLFRA::ModelInterface::CalculationSuccess; }

	// Update state of model to new set of unknowns.
	virtual SOLFRA::ModelInterface::CalculationResult setY(const double * y)
	{std::memcpy(&m_states[0], y , n()*sizeof(double)); }

	// Update f-function hier and fill in ydot.
	virtual SOLFRA::ModelInterface::CalculationResult ydot(double * ydot)
	\endcode

	For initializing steady state calculation initialize start solution
	y0, sparse matrix pattern for KLU solver, all constraints and call
	initSparseSolver()/initDenseSolver.

	\code
	// resize solution variable
	m_states.resize(10,0);

	// resize dummy index
	std::set<unsigned int>          emptySet;
	std::vector<unsigned int>       emptyIndex;

	// initialize sparse matrix pattern
	IBKMK::SparseMatrixPattern pattern;
	// fill sparse matrix pattern
	...
	// get CSR row and column indices
	DAEMatrixPattern::patternCSR(pattern, nStates(), emptySet, m_ia, m_ja, emptyIndex);

	// set optional constrains for eac solution quantity: 1: > 0, -1: < 0, 2: >= 0, -2: <= 0
	std::vector<int> constraints(10, 1);

	// initialize solver (m_iaT, m_jaT are empty vectors that reserve storage for
	// tranpose row and column index)
	initSparseSolver(m_ia, m_ja, m_iaT, m_jaT, constraints);

	\endcode

	For starting steady state calculation at any time
	initialize start solution y0 and call newtonIteration():

	\code
	// fill quantity with a suitable start value
	for(unsigned int i = 0; i < 10; ++i) {
		m_states[i] = 1;
	}
	// let y0() return pointer to first vector element m_states

	// start newton iteration
	int successOfKinsolCall = newtonIteration();

	// enforce solver abort
	if (m_successOfLastKinsolCall == CalculationAbort)
		throw IBK::Exception("Steady state solver aborted!",FUNC_ID);
	// ask solver to try again with smaller time step
	else if (m_successOfLastKinsolCall == CalculationRecoverableError) {
		IBK::IBK_Message(IBK::FormatString("Convergence error of steady state solver. "
		"Try again with updated Jacobain or reduce time step.\n"),
		IBK::MSG_WARNING, FUNC_ID);
		// signal error
		return 1;
	}
	else {
		// signal success: m_states now contains steady states solution
		return 0;
	}
	\endcode
*/

class SteadyStateSolver: public SOLFRA::ModelInterface {
public:
	enum CalculationOptions {
		KinsolStrictNewton = 0x01,
		KinsolLineSearch = 0x02
	};

	/*! Default constructor: we enforce definition of calculation options. */
	SteadyStateSolver();

	/*! Default destructor. */
	~SteadyStateSolver();

	/********* Functions inherited from ModelInterface. ********/

	/*! Number of solver unknowns. */
	virtual unsigned int n() const;
	/*! Start time point in [s]. */
	virtual double t0() const { return 0.0; };
	/*! Initial time step. */
	virtual double dt0() const { return 0.0; };
	/*! End time point in [s]. */
	virtual double tEnd() const { return 0.0; };
	/*! Returns model-specific Jacobian matrix generation method for use with iterative LES solvers.
	Default implementation returns NULL (no jacobian generator).
	*/
	virtual SOLFRA::JacobianInterface * jacobianInterface();


	/********* Individual functions. ********/

	/*! Constant access to states vector.*/
	const double *states() const { return &m_y[0]; }
	/*! Constanmt access to residuals vector.*/
	const double *residuals() const { return &m_ydot[0]; }
	/*! Constant access to solver options.*/
	unsigned int options() const { return m_options; }
	/*! Constant access to m,aximum nonlinear iteartions.*/
	unsigned int maxNonlinearIter() const { return m_maximumNonlinearIterations; }

	/*! Updates all internal model states and resiudals stores in the vector m_states and m_residuals.
	*/
	int updateStatesAndResiduals(const double * y);
	/*! Calculates the jacobian for a given state.*/
	int updateJacobian(const double * y);

protected:

	/*! Initialition of sparse jacobian and kinsol sparse solver:
		\para ia CSR row indices, predefined by calling class
		\para ja CSR column indices, predefined by calling class
		\para iaT CSR row indices for transpose matrix, will be calculated inside function
		\para jaT CSR column indices for transpose matrix, will be calculated inside function
		*/
	void initSparseSolver(const std::vector<unsigned int> &ia,
		const std::vector<unsigned int> &ja,
		std::vector<unsigned int> &iaT,
		std::vector<unsigned int> &jaT);

	/*! Initialition of dense jacobian and kinsol dense solver:
	\para n problem dimension
	*/
	void initDenseSolver(unsigned int n);

	/*! Optional solver options settings.
	\para options Solver options, bit code of CalculationOptions
	*/
	void setOptions(unsigned int options);

	/*! Optional constraints settings. May only be set after initailization of
		sparse or dense solver. Otherwise, an exception is thrown.
	\para constraints constrains for all solution quantities (> 0, >=0 or none)
	*/
	void setConstraints(const int *constraints);

	/*! Optional set tolerance. May only be set after initailization of
		sparse or dense solver. Otherwise, an exception is thrown.
	\para tolerance Tolerance for y-value and residuals
	*/
	void setTolerance(double tolerance);

	/*! Optional set a value for maximum nonlinear iterations.
	\para maxNonlinIter MAximum number of nonlinear iteration calls
	*/
	void setMaxNonlinearIterations(unsigned int maxNonlinIter);

	/*! Global calculation function.*/
	CalculationResult newtonIteration(const double *yscale = NULL,
									  const double *fscale = NULL);

private:

	/*! The Jacobian is represented by a dense or sparse matrix.*/
	SOLFRA::JacobianInterface				*m_jacobian;
	/*! Kinsol memory pointer. */
	void									*m_kinsolMem;
	/*! Used to store y elements for Matrix setup and Newton iteration. */
	std::vector<double>						m_y;
	/*! Used to store ydot elements for Matrix setup and Newton iteration. */
	std::vector<double>						m_ydot;
	/*! Storage element for kinsol solver. */
	N_Vector								m_yStorageKinsol;
	/*! Relaxation vector of the solution of nonlinear solver. */
	N_Vector								m_yScale;
	/*! Relaxation vector of the rsiduals of nonlinear solver. */
	N_Vector								m_fScale;
	/*! Pointer to outside defined relaxation vector of the solution of nonlinear solver. */
	const double							*m_yScalePtr;
	/*! Pointer to outside defined relaxation vector of the rsiduals of nonlinear solver. */
	const double							*m_fScalePtr;
	/*! Vector that stores all rows that were replaced by a deltaY = 0 condition. Needed for synchronizing residuals vector.*/
	std::vector<bool>						m_zeroRows;
	//	/*! Constraints vector Kinsol. */
	N_Vector								m_constraintsKinsol;
	/*! Maximum number of Newton iterations for a cyclic group. */
	unsigned int							m_maximumNonlinearIterations;
	/*! Solver options. */
	unsigned int 							m_options;

	friend class NandradModelImpl;
	friend class StateModelGroup;
};

} // namespace NANDRAD_MODEL

#endif // NM_SteadyStateSolverH
