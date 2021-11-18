#include "SOLFRA_LESDense.h"

#include <fstream>

#include "SOLFRA_IntegratorSundialsCVODE.h"
#include "SOLFRA_IntegratorImplicitEuler.h"
#include "SOLFRA_ModelInterface.h"
#include "SOLFRA_Constants.h"

#include <IBKMK_DenseMatrix.h>
#include <IBK_Exception.h>
#include <IBK_FormatString.h>
#include <IBK_messages.h>
#include <IBK_InputOutput.h>

#include <cvode/cvode_dense.h>
#include <cvode/cvode_serialization.h>


namespace SOLFRA {

LESDense::LESDense() :
	m_jacobian(nullptr)
{
}

LESDense::~LESDense() {
	delete m_jacobian;
}


void LESDense::init(ModelInterface * model, IntegratorInterface * integrator,
					PrecondInterface * precond, JacobianInterface * /*jacobian*/)
{
	FUNCID(LESDense::init);

	// retreive pointer to the current model
	m_model		 = model;
	// retreive pointer to the current integrator
	m_integrator = integrator;

	// determine which preconditioner we are using
	if(precond != nullptr)
		IBK::IBK_Message( "LESDense solver is chosen: Preconditioner will be ignored.", IBK::MSG_WARNING, FUNC_ID);

	// determine which integrator we are using
	if (IntegratorSundialsCVODE* intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(m_integrator)) {
		// create and register CVDense with integrator
		int result = CVDense(intCVODE->cvodeMem(), model->n());
		if (result != CVDLS_SUCCESS) {
			if (result == CVDLS_MEM_FAIL)
				IBK::IBK_Message( "CVDense memory initialization error (problem too large?)", IBK::MSG_ERROR, FUNC_ID);
			else if (result == CVDLS_ILL_INPUT)
				IBK::IBK_Message( "CVDense init error (wrong input?)", IBK::MSG_ERROR, FUNC_ID);
			else
				IBK::IBK_Message( "CVDense init error", IBK::MSG_ERROR, FUNC_ID);
			throw IBK::Exception("Error initializing linear equation solver.", FUNC_ID);
		}
		return;
	}
	else if (IntegratorImplicitEuler* intEuler = dynamic_cast<IntegratorImplicitEuler*>(m_integrator)) {
		(void)intEuler;
		// for now, only resize matrix and vectors when using implicit Euler (so that we are not wasting memory)
		m_n = model->n();
		m_jacobian = new IBKMK::DenseMatrix(m_n);
		m_yMod.resize(m_n);
		m_ydotMod.resize(m_n);
		m_ydiff.resize(m_n);

		return;
	}

	throw IBK::Exception("Error initialising Dense linear solver: solver is only defined for CVODE, IDA or implicit Euler integrator",FUNC_ID);
}

void LESDense::setup(const double * y, const double * ydot, const double * /* residuals */, double gamma) {
	// only setup linear equation system explicitely if integrator implicit Euler is chosen
	if (dynamic_cast<IntegratorImplicitEuler*>(m_integrator) == nullptr)
		return;

	m_jacobian->fill(0.0);
	// store current solution guess
	std::memcpy(&m_yMod[0], y, m_yMod.size()*sizeof(double));

	// loop over all columns
	for (unsigned int j=0; j<m_n; ++j) {
		// modify all y value at column j
		m_ydiff[j] = std::fabs(y[j])*m_relToleranceDQ + m_absToleranceDQ;
		m_yMod[j] += m_ydiff[j];

		// update model
		if (m_model != nullptr) {
			// calculate modified right hand side
			m_model->setY(&m_yMod[0]);
			// calculate modified right hand side of the model, and store f(t,y) in m_FMod
			m_model->ydot(&m_ydotMod[0]);
			// F = y - yn - dt*ydot,
			// derivative: 1 - dt * dydot/dy
			for (unsigned int i=0; i<m_n; ++i) {
				// compute finite-differences column j in row i
				double val = ( m_ydotMod[i] - ydot[i] )/m_ydiff[j];
				(*m_jacobian)(i,j) = val;
			}
			// Jacobian matrix now holds df/dy
			// update solver statistics
			++m_statNumRhsEvals;
		}
		// restore y vector
		m_yMod[j] = y[j];
		// reset ydiff
		m_ydiff[j] = 0;
	}
#ifdef DUMP_JACOBIAN_TEXT
	std::ofstream jacdump("jacobian_dense.txt");
	m_jacobian->write(jacdump, nullptr, false, 15);
	jacdump.close();
	throw IBK::Exception("Done with test-dump of Jacobian", "[LESDense::setup]");
#endif
#ifdef DUMP_JACOBIAN_BINARY
	IBK::write_matrix_binary(*m_jacobian, "jacobian_dense.bin");
	throw IBK::Exception("Done with test-dump of Jacobian", "[LESDense::setup]");
#endif

	// scale matrix with -gamma
	unsigned int dataSize = m_n*m_n;
	for (unsigned int i=0; i<dataSize; ++i)
		m_jacobian->data()[i] *= -gamma;
	// add identity matrix
	for (unsigned int i=0; i<m_n; ++i)
		(*m_jacobian)(i,i) += 1.0;

	// perform LU-factorisation of the dense jacobian
	m_jacobian->lu();
	// update solver statistics
	++m_statNumJacEvals;
}


void LESDense::solve(double * rhs) {
	// only solve linear equation system explicitely if integrator implicit Euler is chosen
	if (dynamic_cast<IntegratorImplicitEuler*>(m_integrator) == nullptr)
		return;

	// backsolve with given lu factorisation
	m_jacobian->backsolve(rhs);
}


std::size_t LESDense::serializationSize() const {
	// determine which integrator we are using
	if (IntegratorSundialsCVODE* intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(m_integrator)) {
		// we are using the CVSpgmr solver
		std::size_t s = CVDlsSerializationSize(intCVODE->cvodeMem());
		return s;
	}
	else {
		 return SOLFRA_NOT_SUPPORTED_FUNCTION; // serialization not supported
	}
}


void LESDense::serialize(void *& dataPtr) const {
	// determine which integrator we are using
	if (IntegratorSundialsCVODE* intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(m_integrator)) {
		CVDlsSerialize(intCVODE->cvodeMem(), &dataPtr); // ignoring return value for now
	}
}


void LESDense::deserialize(void *& dataPtr) {
	// determine which integrator we are using
	if (IntegratorSundialsCVODE* intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(m_integrator)) {
		CVDlsDeserialize(intCVODE->cvodeMem(), &dataPtr); // ignoring return value for now
	}
}


} // namespace SOLFRA

