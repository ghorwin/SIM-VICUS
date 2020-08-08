/*	The Nandrad model library.

Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
*/

#include "NM_SteadyStateSolver.h"

//#include <SOLFRA_JacobianDense.h>
#include <SOLFRA_JacobianSparseCSR.h>

#include <IBK_assert.h>
#include <IBK_math.h>
#include <IBK_messages.h>

#include <IBKMK_SparseMatrixPattern.h>

#include <algorithm>
#include <limits>

#include <sundials/sundials_direct.h>
#include <kinsol/kinsol.h>
#include <kinsol/kinsol_dense.h>
#include <kinsol/kinsol_klu.h>


// OpenMP NVector only makes sense for really really huge vectors,
// and for medium sized and small vectors (n<250000) it harms performance
#if 0 && defined(_OPENMP)
#include <nvector/nvector_openmp.h>
#define N_VNew(x) N_VNew_OpenMP(x, m_numThreads)
#define NV_DATA(y) NV_DATA_OMP(y)
#define N_VDestroy(z) N_VDestroy_OpenMP(z)
#else
#include <nvector/nvector_serial.h>
#define N_VNew(x) N_VNew_Serial(x)
#define NV_DATA(y) NV_DATA_S(y)
#define N_VDestroy(z) N_VDestroy_Serial(z)
#endif


namespace NANDRAD_MODEL {

/*! Wrapper function called from Kinsol solver which calls the actual solver routine in
the solver class.
\param y Estimated solution vector at current solution state.
\param res Vector for the calculated residuals.
\param user_data Pointer to the IntegratorSundialsCVODEImpl object.
*/
inline int KINSysFn_f(N_Vector y, N_Vector res, void *user_data) {
	SteadyStateSolver * stateModel = static_cast<SteadyStateSolver*>(user_data);
	IBK_ASSERT(stateModel != NULL);
	// set y value and calculate corresponding residuals
	int result = stateModel->updateStatesAndResiduals(NV_DATA(y));
	// return error
	if (result > 0)
		return result;
	// copy residuals
	std::memcpy(NV_DATA(res), stateModel->residuals(), stateModel->n() * sizeof(double));
	return 0; // Success
}
// ---------------------------------------------------------------------------

/*! Wrapper function called from Kinsol solver for calculation of the Jacobian.
We need this function in order to specially treat singular matrix entries.
\param y Estimated solution vector at current solution state.
\param res Residual vector at current solution state.
\param J Jacobian copy of current system.
\param user_data Pointer to the IntegratorSundialsCVODEImpl object.
*/
inline int KINDlsDenseJacFn_f(long int n, N_Vector y, N_Vector res,
	DlsMat J, void *user_data,
	N_Vector tmp1, N_Vector tmp2) {

	SteadyStateSolver * stateModel = static_cast<SteadyStateSolver*>(user_data);
	IBK_ASSERT(stateModel != NULL);

	// update jacobian
	int result = stateModel->updateJacobian(NV_DATA(y));
	// return error
	if (result > 0)
		return result;
#if 0
	// we only accept dense jacobian
	SOLFRA::JacobianDense* jacDense = dynamic_cast<SOLFRA::JacobianDense*> (stateModel->jacobianInterface());
	IBK_ASSERT(jacDense != NULL);

	// copy jacobian data
	for (unsigned int i = 0; i < stateModel->n(); ++i) {
		for (unsigned int j = 0; j < stateModel->n(); ++j) {
			DENSE_ELEM(J,i,j) = (*jacDense)(i, j);
		}
	}
#endif
	return 0; // Success
}
// ---------------------------------------------------------------------------


/*! Wrapper function called from Kinsol solver for calculation of the Jacobian.
We need this function in order to specially treat singular matrix entries.
\param y Estimated solution vector at current solution state.
\param res Residual vector at current solution state.
\param Jac Jacobian copy of current system.
\param user_data Pointer to the IntegratorSundialsCVODEImpl object.
*/
inline int KINSlsSparseJacFn_f(N_Vector y, N_Vector res,
	SlsMat J, void *user_data,
	N_Vector tmp1, N_Vector tmp2) {

	SteadyStateSolver * stateModel = static_cast<SteadyStateSolver*>(user_data);
	IBK_ASSERT(stateModel != NULL);

	// update y vector and dense Jacobian
	stateModel->setY(NV_DATA(y));
	// update jacobian
	stateModel->updateJacobian(NV_DATA(y));

	// we only accept sparse jacobian CSR
	SOLFRA::JacobianSparseCSR* jacSparse = dynamic_cast<SOLFRA::JacobianSparseCSR*> (stateModel->jacobianInterface());
	IBK_ASSERT(jacSparse != NULL);
	const IBKMK::SparseMatrixCSR *sparseMat = dynamic_cast<const IBKMK::SparseMatrixCSR*> (jacSparse->jacobian());
	IBK_ASSERT(sparseMat != NULL);
	// set elements to zero
	SparseSetMatToZero(J);

	// no data
	IBK_ASSERT(sparseMat->dataSize() > 0);
	// copy jacobian data:
	IBK_ASSERT(sparseMat->n() > 0);
	IBK_ASSERT(sparseMat->nnz() > 0);
	// initialization necessary
	if ((*J->rowptrs)[1] != (int) sparseMat->constIa()[1]) {
		// row data
		for (unsigned int i = 0; i < sparseMat->n() + 1; ++i) {
			(*J->rowptrs)[i] = (int)sparseMat->constIa()[i];
		}
		// column data
		for (unsigned int j = 0; j < sparseMat->nnz(); ++j) {
			(*J->colvals)[j] = (int) sparseMat->constJa()[j];
		}
	}
	// matrix data
	for (unsigned int j = 0; j < sparseMat->nnz(); ++j) {
		J->data[j] = sparseMat->data()[j];
	}
	return 0; // Success
}

// ---------------------------------------------------------------------------
/*! Wrapper function called from Kinsol solver when Kinso,l convergecne errors occur.
We allow more than fails of a single Kinsol step in order to trigger Jacobian setup.
KinSol error messages will be monitored for high verbosity level
*/
inline void KINErrHandlerFn_f(int code, const char *module,
	const char *function, char *msg,
	void * /*data*/) {

	std::string funcCode = std::string("[") + std::string(module) +
		std::string("::") + std::string(function) + std::string("]");
	const char* const FUNC_ID = funcCode.c_str();

	IBK::IBK_Message(IBK::FormatString("[%1::%2] KinSol error: %3")
		.arg(module).arg(function).arg(msg), IBK::MSG_WARNING, FUNC_ID, 2);
}
// ---------------------------------------------------------------------------


// *** Implementation of SteadyStateSolver ***

SteadyStateSolver::SteadyStateSolver() :
	m_jacobian(NULL),
	m_kinsolMem(NULL),
	m_yScalePtr(NULL),
	m_fScalePtr(NULL),
	m_maximumNonlinearIterations(2),
	m_options(0)
{
}

SteadyStateSolver::~SteadyStateSolver(){
	if(m_jacobian != NULL)
		delete m_jacobian;
}


void SteadyStateSolver::initSparseSolver(const std::vector<unsigned int> &ia,
										const std::vector<unsigned int> &ja,
										std::vector<unsigned int> &iaT,
										std::vector<unsigned int> &jaT)
{
	const char * const FUNC_ID = "[SteadyStateSolver::initSparseSolver]";

	// resize all vectors
	m_y.resize(ia.size() - 1, 0.0);
	m_ydot.resize(ia.size() - 1, 0.0);
	m_zeroRows.resize(ia.size() - 1, false);

	if(iaT.empty() || jaT.empty())
		// generate sparse matrix index
		IBKMK::SparseMatrixCSR::generateTransposedIndex(ia, ja, iaT, jaT);
	m_jacobian = new SOLFRA::JacobianSparseCSR((unsigned int) ia.size() - 1 , (unsigned int) ja.size(), &ia[0], &ja[0],
		&iaT[0], &jaT[0]);
	m_jacobian->init(this);


	// initialize Kinsol solver
	// Initialize kinsol memory
	m_kinsolMem = KINCreate();
	// resize kinsol storage vector
	m_yStorageKinsol = N_VNew((long) ia.size() - 1);
	//	initialize relaxation vector
	m_yScale = N_VNew_Serial((long)ia.size() - 1);
	m_fScale = N_VNew_Serial((long)ia.size() - 1);
	N_VConst_Serial(1, m_yScale);
	N_VConst_Serial(1, m_fScale);

	// Initialize Kinsol memory with right hand side function
	int result = KINInit(m_kinsolMem, KINSysFn_f, m_yStorageKinsol);
	if (result != KIN_SUCCESS)
		throw IBK::Exception("KINInit init error.", FUNC_ID);

	// *** set the Kinsol options ***
	// set CVODE data pointer to solver object
	KINSetUserData(m_kinsolMem, (void*)this);

	// set Kinsol maximum steps before stopping with error emssage
	KINSetMaxSetupCalls(m_kinsolMem, 3);
	KINSetMaxBetaFails(m_kinsolMem, 100);
	KINSetScaledStepTol(m_kinsolMem, 1e-09);

	// initialize sparse solver module
	result = KINKLU(m_kinsolMem, ((int) ia.size() - 1), (int) ja.size(), CSR_MAT);
	if (result != KINSLS_SUCCESS) {
		if (result == KINSLS_MEM_FAIL)
			IBK::IBK_Message("KINKLU memory initialization error (problem too large?)", IBK::MSG_ERROR, FUNC_ID);
		else if (result == KINSLS_ILL_INPUT)
			IBK::IBK_Message("KINKLU init error (wrong input?)", IBK::MSG_ERROR, FUNC_ID);
		else
			IBK::IBK_Message("KINKLU init error", IBK::MSG_ERROR, FUNC_ID);
		throw IBK::Exception("Error in setting up linear equation solver module.", FUNC_ID);
	}
	// set Kinsol sparse Jacobian setup function
	result = KINSlsSetSparseJacFn(m_kinsolMem, KINSlsSparseJacFn_f);
	IBK_ASSERT(result == KINSLS_SUCCESS);
	// set error handler funcion
	KINSetErrHandlerFn(m_kinsolMem, KINErrHandlerFn_f, NULL);
	IBK_ASSERT(result == KINSLS_SUCCESS);

	// initialize local solution
	std::memcpy(&m_y[0], y0(), (ia.size() - 1) * sizeof(double));
}



void SteadyStateSolver::initDenseSolver(unsigned int n) {
#if 0
	const char * const FUNC_ID = "[SteadyStateSolver::initDenseSolver]";
	// resize all vectors
	m_y.resize(n, 0.0);
	m_ydot.resize(n, 0.0);
	m_zeroRows.resize(n, false);
	// initialze dense jacobian
	m_jacobian = new SOLFRA::JacobianDense;
	m_jacobian->init(this);

	// initialize Kinsol solver
	// Initialize kinsol memory
	m_kinsolMem = KINCreate();
	// resize kinsol storage vector
	m_yStorageKinsol = N_VNew(n);
	//	initialize relaxation vector
	m_yScale = N_VNew_Serial(n);
	m_fScale = N_VNew_Serial(n);
	N_VConst_Serial(1, m_yScale);
	N_VConst_Serial(1, m_fScale);

	// Initialize Kinsol memory with right hand side function
	int result = KINInit(m_kinsolMem, KINSysFn_f, m_yStorageKinsol);
	if (result != KIN_SUCCESS)
		throw IBK::Exception("KINInit init error.", FUNC_ID);

	// *** set the Kinsol options ***
	// set CVODE data pointer to solver object
	KINSetUserData(m_kinsolMem, (void*)this);

	// set Kinsol maximum steps before stopping with error emssage
	KINSetMaxSetupCalls(m_kinsolMem, m_maximumNonlinearIterations);
	KINSetMaxBetaFails(m_kinsolMem, 100);
	KINSetScaledStepTol(m_kinsolMem, 1e-09);

	// enable dense kinsol linear equation solver
	result = KINDense(m_kinsolMem, n);
	if (result != KIN_SUCCESS)
		throw IBK::Exception("KINDense error in setting up linear equation solver module.", FUNC_ID);
	// set Kinsol dense Jacobian setup function
	result = KINDlsSetDenseJacFn(m_kinsolMem, KINDlsDenseJacFn_f);
	if (result != KIN_SUCCESS)
		throw IBK::Exception("KINDense error in setting Jacobian function.", FUNC_ID);
	// set error handler funcion
	KINSetErrHandlerFn(m_kinsolMem, KINErrHandlerFn_f, NULL);
	IBK_ASSERT(result == KINSLS_SUCCESS);

	// initialize local solution
	std::memcpy(&m_y[0], y0(), n * sizeof(double));
#endif
}


void SteadyStateSolver::setOptions(unsigned int options) {

	const char * const FUNC_ID = "[SteadyStateSolver::setOptions]";
	if (m_kinsolMem == NULL)
	{
		throw IBK::Exception("setOptions only may be called after initSparseSolver or initDenseSolver!",
			FUNC_ID);
	}
	m_options = options;
	if (m_options & KinsolStrictNewton) {
		// disable modified Newton
		KINSetNoResMon(m_kinsolMem, TRUE);
	}
	else {
		// enable modified Newton
		KINSetNoResMon(m_kinsolMem, FALSE);
	}
}


void SteadyStateSolver::setConstraints(const int *constraints) {

	const char * const FUNC_ID = "[SteadyStateSolver::setConstraints]";
	if (m_kinsolMem == NULL)
	{
		throw IBK::Exception("setConstraints only may be called after initSparseSolver or initDenseSolver!",
			FUNC_ID);
	}
	// ertrieve problem size
	IBK_ASSERT(!m_y.empty());
	unsigned int n = (unsigned int) m_y.size();
	// lenght 0 signals no constraints
	if (constraints != NULL) {
		m_constraintsKinsol = N_VNew_Serial(n);
		N_VConst_Serial(0, m_constraintsKinsol);
		// copy constraint
		for (unsigned int i = 0; i < n; ++i) {
			NV_DATA(m_constraintsKinsol)[i] = constraints[i];
		}
		KINSetConstraints(m_kinsolMem, m_constraintsKinsol);
	}
}


void SteadyStateSolver::setTolerance(double tolerance) {

	const char * const FUNC_ID = "[SteadyStateSolver::setTolerance]";
	if (m_kinsolMem == NULL)
	{
		throw IBK::Exception("setTolerance only may be called after initSparseSolver or initDenseSolver!",
			FUNC_ID);
	}
	// set tolerances
	KINSetScaledStepTol(m_kinsolMem, tolerance);
	KINSetFuncNormTol(m_kinsolMem, tolerance);
}


void SteadyStateSolver::setMaxNonlinearIterations(unsigned int maxNonlinIter) {
	// set iteration number
	m_maximumNonlinearIterations = maxNonlinIter;
}


SOLFRA::ModelInterface::CalculationResult SteadyStateSolver::newtonIteration(const double *yscale,
																			 const double *fscale) {

	const char * const FUNC_ID = "[SteadyStateSolver::newtonIteration]";
	// if no jacobian is defined set jacobian and solver to dense
	if (m_jacobian == NULL) {
		throw IBK::Exception(IBK::FormatString("Missing linear equation solver "
			"in SteadyStateSolver: call 'initDenseSolver' or 'initSparseSolver' first!"),
			FUNC_ID);
	}

	// get initial states
	std::memcpy( &m_y[0], y0(), n() * sizeof(double));
	// transfer initial solution for kinsol run
	std::memcpy(NV_DATA(m_yStorageKinsol), &m_y[0], n() * sizeof(double));

	if (yscale != NULL) {
		// copy factors
		for (unsigned int i = 0; i < n(); ++i) {
			NV_DATA(m_yScale)[i] = yscale[i];
		}
	}
	if (fscale != NULL) {
		// copy factors
		for (unsigned int i = 0; i < n(); ++i) {
			NV_DATA(m_fScale)[i] = fscale[i];
		}
	}

	int successOfLastKinsolCall = 0;

	if (m_options & KinsolLineSearch) {
		// start linear solution
		// Retry after a failed step for new setup of linear system
		for (unsigned int i = 0; i < m_maximumNonlinearIterations; ++i) {
			// start linear solution
			successOfLastKinsolCall = KINSol(m_kinsolMem, m_yStorageKinsol, KIN_LINESEARCH, m_yScale, m_fScale);
			if (successOfLastKinsolCall == KIN_SUCCESS ||
				successOfLastKinsolCall == KIN_INITIAL_GUESS_OK)
				break;
		}
	}
	else {
		// if call was not successful, try with direct Newton
		for (unsigned int i = 0; i < m_maximumNonlinearIterations; ++i) {
			// start linear solution
			successOfLastKinsolCall = KINSol(m_kinsolMem, m_yStorageKinsol, KIN_NONE, m_yScale, m_fScale);
			if (successOfLastKinsolCall == KIN_SUCCESS ||
				successOfLastKinsolCall == KIN_INITIAL_GUESS_OK)
				break;
		}
	}

	// if call was not successful, return an invalid solution
	if (successOfLastKinsolCall != KIN_SUCCESS &&
		successOfLastKinsolCall != KIN_INITIAL_GUESS_OK) {
		// signal error
		return CalculationRecoverableError;
	}
	else {
		// copy solution to model variables
		std::memcpy(&(m_y[0]), NV_DATA_S(m_yStorageKinsol), m_y.size()*sizeof(double));
		// udpate model states and residuals
		setY(&m_y[0]);
		return CalculationSuccess;
	}
}


unsigned int SteadyStateSolver::n() const {
	return (unsigned int) m_y.size();
}

SOLFRA::JacobianInterface * SteadyStateSolver::jacobianInterface() {
	return m_jacobian;
}


int SteadyStateSolver::updateStatesAndResiduals(const double * y) {
	std::memcpy(&m_y[0], y, n() * sizeof(double));
	// udpate states
	setY(y);
	// set residuals equal to ydot
	SOLFRA::ModelInterface::CalculationResult res = ydot(&m_ydot[0]);

	for (unsigned int i = 0; i < n(); ++i) {
		if (m_zeroRows[i] == true) {
			//set residual to zero
			m_ydot[i] = 0;
		}
	}

	switch (res) {
	case SOLFRA::ModelInterface::CalculationAbort:
		return 2;
		break;
	case SOLFRA::ModelInterface::CalculationRecoverableError:
		return 1;
		break;
	case SOLFRA::ModelInterface::CalculationSuccess:
		return 0;
		break;
	default:
		return 0;
		break;
	}
}


int SteadyStateSolver::updateJacobian(const double * y) {
#if 0
	// start updating residuals for new Jacobian calculation
	std::fill(m_zeroRows.begin(), m_zeroRows.end(), false);
	// get current state and calculate residuals
	int res = updateStatesAndResiduals(y);

	if (res > 0)
		return res;
	// setup jacobian
	jacobianInterface()->setup(0.0, &m_y[0], &m_ydot[0], NULL, 0.0);
	// get access to dense jacobian
	SOLFRA::JacobianDense* jacDense = dynamic_cast<SOLFRA::JacobianDense*> (jacobianInterface());
	// get access to sparse jacobian
	SOLFRA::JacobianSparseCSR* jacSparse = dynamic_cast<SOLFRA::JacobianSparseCSR*> (jacobianInterface());
	IBK_ASSERT(jacDense != NULL || jacSparse != NULL);
	// identify state events (zero rows in matrix)
	// and set diagonal to 1 (no change of quantity is enforced)
	if (jacDense != NULL) {
		for (unsigned int i = 0; i < n(); ++i)
		{
			bool zeroRow = true;
			for (unsigned int j = 0; j < n(); ++j)
			{
				if (std::fabs((*jacDense)(i, j)) > 1e-15) {
					zeroRow = false;
					break;
				}
			}
			// enforce deltaY[j] = 0
			if (zeroRow) {
				(*jacDense)(i, i) = 1.0;
				m_zeroRows[i] = true;
				// set residual to 0
				m_ydot[i] = 0;
			}
		}
	}
	// the same for sparse matrix
	else {
		IBKMK::SparseMatrix *sparseMat;
		// complicated cast because of access problems to jacobian
		sparseMat = const_cast<IBKMK::SparseMatrix *> (jacSparse->jacobian());
		IBKMK::SparseMatrixCSR *sparseMatCSR = dynamic_cast<IBKMK::SparseMatrixCSR *>(sparseMat);
		// calidity checks
		IBK_ASSERT(sparseMatCSR != NULL);
		IBK_ASSERT(sparseMatCSR->n() > 0);
		for (unsigned int i = 0; i < sparseMatCSR->n(); ++i)
		{
			bool zeroRow = true;
			// find diagonal element
			for (unsigned int jIdx = sparseMatCSR->constIa()[i];
				jIdx < sparseMatCSR->constIa()[i + 1]; ++jIdx)
			{
				// set diagonal element
				if (std::fabs(sparseMatCSR->data()[jIdx]) > 1e-15)
					zeroRow = false;
			}
			// enforce deltaY[j] = 0
			if (zeroRow) {
				(*sparseMatCSR)(i, i) = 1.0;
				m_zeroRows[i] = true;
				// set residual to 0
				m_ydot[i] = 0;
			}
		}

	}
	// signal success
	return res;
#endif
	return 0;
}

} // namespace NANDRAD_MODEL

