#include "SOLFRA_LESKLU.h"

#include <IBKMK_SparseMatrixCSR.h>
#include <IBK_messages.h>

#include <cvode/cvode_klu.h>
#include <cvode/cvode_sparse.h>
#include <nvector/nvector_serial.h>
#include <sundials/sundials_timer.h>

#include "SOLFRA_IntegratorSundialsCVODE.h"
#include "SOLFRA_JacobianSparseCSR.h"
#include "SOLFRA_ModelInterface.h"
#include "SOLFRA_IntegratorSundialsCVODEImpl.h"

namespace SOLFRA {

/*! Wrapper function called from CVODE solver for calculation of the Jacobian
	df/dy. This function copies sparse matrix indices at first call. Further,
	it assembles Jacobian CSR and copies into CVODE internal matrix structure.
	\param y Estimated solution vector at current solution state.
	\param ydot Divergences for current solution state.
	\param Jac Jacobian copy of current system.
	\param user_data Pointer to the IntegratorSundialsCVODEImpl object.
*/
inline int CVSlsSparseJacFn_f(realtype t, N_Vector y, N_Vector ydot,
	SlsMat J, void *user_data,
	N_Vector tmp1, N_Vector tmp2, N_Vector tmp3)
{
	(void)tmp1;(void)tmp2; (void)tmp3;

	// Note: Matrix J was already zeroed by CVODE

	IntegratorSundialsCVODEImpl * cvodeWrapper = static_cast<IntegratorSundialsCVODEImpl*>(user_data);
	ModelInterface * model = cvodeWrapper->m_model;
	IBK_ASSERT(model != nullptr);

	// update y, ydot vectors and sparse Jacobian
	SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_FEVAL_JACOBIAN_GENERATION,
		model->setY(NV_DATA_S(y));
		model->ydot(NV_DATA_S(ydot));
	);

	LESInterfaceDirect * lesSolver = dynamic_cast<LESInterfaceDirect *>(model->lesInterface());
	++(lesSolver->m_statNumRhsEvals);

	double gamma = model->integratorInterface()->dt();
	model->jacobianInterface()->setup(t, NV_DATA_S(y), NV_DATA_S(ydot), nullptr,
		 gamma);

	// we only accept sparse jacobian CSR
	SOLFRA::JacobianSparseCSR* jacSparse = dynamic_cast<SOLFRA::JacobianSparseCSR*>
		(model->jacobianInterface());
	IBK_ASSERT(jacSparse != nullptr);
	const IBKMK::SparseMatrixCSR *sparseMat = dynamic_cast<const IBKMK::SparseMatrixCSR*>
		(jacSparse->jacobian());
	IBK_ASSERT(sparseMat != nullptr);

	// no data
	IBK_ASSERT(sparseMat->dataSize() > 0);
	// copy jacobian data:
	IBK_ASSERT(sparseMat->n() > 0);
	IBK_ASSERT(sparseMat->nnz() > 0);
	// initialization necessary
	if ((*J->rowptrs)[1] != (int)sparseMat->constIa()[1]) {
		// row data
		for (unsigned int i = 0; i < sparseMat->n() + 1; ++i) {
			(*J->rowptrs)[i] = sparseMat->constIa()[i];
		}
		// column data
		for (unsigned int j = 0; j < sparseMat->nnz(); ++j) {
			(*J->colvals)[j] = sparseMat->constJa()[j];
		}
	}
	// copy matrix data into cvode sparse matrix structure
	for (unsigned int j = 0; j < sparseMat->nnz(); ++j) {
		J->data[j] = sparseMat->data()[j];
	}

	return 0; // Success
}
// ---------------------------------------------------------------------------


LESKLU::LESKLU() :
	m_jacobian(nullptr)
{
}

LESKLU::~LESKLU() {
}

void LESKLU::init(ModelInterface * model, IntegratorInterface * integrator,
				   PrecondInterface * precond, JacobianInterface *jacobian)
{
	FUNCID(LESKLU::init);

	m_integrator = integrator;
	m_model = model;

	if (model == nullptr)
		throw IBK::Exception("Missing model.", FUNC_ID);

	// determine which preconditioner we are using
	if (precond != nullptr)
		IBK::IBK_Message( "LESKLU linear solver is chosen: Preconditioner will be ignored.", IBK::MSG_WARNING, FUNC_ID);

	if (jacobian == nullptr) {
		throw IBK::Exception("Error initializing LESKLU linear solver: missing Jacobian.", FUNC_ID);
	}
	// copy jacobian
	JacobianSparseCSR *jacSparse = dynamic_cast<JacobianSparseCSR *>(jacobian);
	if (jacSparse == nullptr) {
		throw IBK::Exception("Error initializing LESKLU linear solver: only JacobianSparseCSR  is allowed.", FUNC_ID);
	}
	m_jacobian = jacSparse;

	// determine which integrator we are using
	if (IntegratorSundialsCVODE* intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(m_integrator)) {
		// get access to all sparse matrix CSR functions
		const IBKMK::SparseMatrix *sparseMat = m_jacobian->jacobian();
		// create and register CVKLU with integrator
		int result = CVKLU(intCVODE->cvodeMem(), model->n(), sparseMat->dataSize(), CSR_MAT);
		if (result != CVSLS_SUCCESS) {
			if (result == CVSLS_MEM_FAIL)
				IBK::IBK_Message( "CVKLU memory initialization error (problem too large?)", IBK::MSG_ERROR, FUNC_ID);
			else if (result == CVSLS_ILL_INPUT)
				IBK::IBK_Message( "CVKLU init error (wrong input?)", IBK::MSG_ERROR, FUNC_ID);
			else
				IBK::IBK_Message( "CVKLU init error", IBK::MSG_ERROR, FUNC_ID);
			throw IBK::Exception("Error initializing linear equation solver.", FUNC_ID);
		}
		// set jacobian calculation function
		result = CVSlsSetSparseJacFn(intCVODE->cvodeMem(), CVSlsSparseJacFn_f);
		IBK_ASSERT(result == CVSLS_SUCCESS);
	}
	else {
		throw IBK::Exception("Error initializing KLU linear solver: solver is only "
			"defined for CVODE integrator",FUNC_ID);
	}
}


void LESKLU::updateSUNDIALSStatistics() {
	if (IntegratorSundialsCVODE* intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(m_integrator)) {
		long statDummy;
		CVSlsGetNumJacEvals(intCVODE->cvodeMem(), &statDummy);
		m_statNumJacEvals = statDummy;
		m_statNumRhsEvals = m_jacobian->nRHSEvals();
	}
}


size_t LESKLU::serializationSize() const {
	// we only need to serialize the actual data and statistics
	return 2 * sizeof(unsigned int);
}


void LESKLU::serialize(void *& dataPtr) const {
	*(unsigned int*)dataPtr = m_statNumJacEvals;
	dataPtr = (char*)dataPtr + sizeof(unsigned int);
	*(unsigned int*)dataPtr = m_statNumRhsEvals;
	dataPtr = (char*)dataPtr + sizeof(unsigned int);
}


void LESKLU::deserialize(void *& dataPtr) {
	m_statNumJacEvals = *(unsigned int*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(unsigned int);
	m_statNumRhsEvals = *(unsigned int*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(unsigned int);
	IntegratorSundialsCVODE* intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(m_integrator);
	if (intCVODE != nullptr)
		intCVODE->setLinearSetupFrequency(1);
}

} // namespace SOLFRA

