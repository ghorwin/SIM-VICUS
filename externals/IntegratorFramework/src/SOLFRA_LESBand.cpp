#include "SOLFRA_LESBand.h"

#include "SOLFRA_IntegratorSundialsCVODE.h"
#include "SOLFRA_IntegratorImplicitEuler.h"
#include "SOLFRA_ModelInterface.h"
#include "SOLFRA_Constants.h"

#include <fstream>
#include <iomanip>
#include <algorithm>

#include <IBKMK_BandMatrix.h>
#include <IBK_messages.h>
#include <IBK_InputOutput.h>

#include <cvode/cvode_band.h>
#include <cvode/cvode_serialization.h>

#if SUNDIALS_BLAS_LAPACK
  #include <cvode/cvode_lapack.h>
#endif // SUNDIALS_BLAS_LAPACK

namespace SOLFRA {

LESBand::LESBand(unsigned mu, unsigned int ml) :
	m_mu(mu),
	m_ml(ml),
	m_jacobian(nullptr)
{
}

LESBand::~LESBand() {
	delete m_jacobian;
}

void LESBand::init(ModelInterface * model, IntegratorInterface * integrator,
				   PrecondInterface * precond, JacobianInterface *jacobian)
{
	const char * const FUNC_ID = "[LESBand::init]";

	m_integrator = integrator;
	m_model = model;

	if (model == nullptr)
		throw IBK::Exception("Missing model.", FUNC_ID);

	// determine which preconditioner we are using
	if (precond != nullptr)
		IBK::IBK_Message( "LESBand linear solver is chosen: Preconditioner will be ignored.", IBK::MSG_WARNING, FUNC_ID);
	if (jacobian != nullptr)
		IBK::IBK_Message( "LESBand linear solver is chosen: Jacobian will be ignored.", IBK::MSG_WARNING, FUNC_ID);

	// determine which integrator we are using
	if (IntegratorSundialsCVODE* intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(m_integrator)) {
		// create and register CVBand with integrator
#if SUNDIALS_BLAS_LAPACK
		int result = CVLapackBand(intCVODE->cvodeMem(), model->n(), m_mu, m_ml);
#else // SUNDIALS_BLAS_LAPACK
		int result = CVBand(intCVODE->cvodeMem(), model->n(), m_mu, m_ml);
#endif // SUNDIALS_BLAS_LAPACK
		if (result != CVDLS_SUCCESS) {
			if (result == CVDLS_MEM_FAIL)
				IBK::IBK_Message( "CVBand memory initialization error (problem too large?)", IBK::MSG_ERROR, FUNC_ID);
			else if (result == CVDLS_ILL_INPUT)
				IBK::IBK_Message( "CVBand init error (wrong input?)", IBK::MSG_ERROR, FUNC_ID);
			else
				IBK::IBK_Message( "CVBand init error", IBK::MSG_ERROR, FUNC_ID);
			throw IBK::Exception("Error initializing linear equation solver.", FUNC_ID);
		}
	}
	else if (IntegratorImplicitEuler* intEuler = dynamic_cast<IntegratorImplicitEuler*>(m_integrator)) {

		(void)intEuler;

		// for now, only resize matrix and vectors when using implicit Euler (so that we are not wasting memory)
		unsigned int n = model->n();
		m_jacobian = new IBKMK::BandMatrix(n, m_mu, m_ml, std::min(n - 1, m_ml + m_mu));
		m_yMod.resize(n);
		m_ydotMod.resize(n);
		m_FMod.resize(n);
		m_ydiff.resize(n);

	}
	else {
		throw IBK::Exception("Error initializing Band linear solver: solver is only "
			"defined for CVODE, IDA or implicit Euler integrator",FUNC_ID);
	}
}


void LESBand::setup(const double * y, const double * ydot, const double * residuals, double gamma) {

	(void) residuals;

//	const char * const FUNC_ID = "[LESBand::setup]";

	// only setup linear equation system explicitely if integrator implicit Euler is chosen
	IBK_ASSERT(dynamic_cast<IntegratorImplicitEuler*>(m_integrator));

	m_jacobian->setZero();
	unsigned int n = m_model->n();

	// store current solution guess
	std::memcpy(&m_yMod[0], y, n*sizeof(double));

	// calculate modification values for each variable and
	for (unsigned int i=0; i<n; ++i) {
		// modify all y value in row i
		m_ydiff[i] = std::fabs(y[i])*m_relToleranceDQ + m_absToleranceDQ;
	}


	// loop over all groups
	unsigned int m = std::min(n, m_ml + m_mu + 1);

	// Use Curtis-Powel-Reid algorithm, modify y in groups
	for (unsigned int i=0; i<m; ++i) {

		// modify y[] with stride m
		for (unsigned int j=i; j<n; j += m) {
			m_yMod[j] += m_ydiff[j];
		}

		// calculate modified right hand side
		m_model->setY(&m_yMod[0]);
		// calculate modified right hand side of the model, and store f(t,y) in m_FMod
		m_model->ydot(&m_FMod[0]);

		// compute Jacobian elements in groups
		for (unsigned int j=i; j<n; j += m) {
			// restrict jacobian assembly to the filled positions of the band
			unsigned int kl = (unsigned int)std::max( 0, (int)(j - m_mu));
			unsigned int ku = (unsigned int)std::min(j + m_ml, n - 1 );
			// F = y - yn - dt*ydot,
			// derivative: 1 - dt * dydot/dy
			for (unsigned int k=kl; k<=ku; ++k) {
				// compute finite-differences column j in row i
				(*m_jacobian)(k,j) =  ( m_FMod[k] - ydot[k] )/m_ydiff[j];
			}
		}
		// Jacobian matrix now holds df/dy
		// update solver statistics
		++m_statNumRhsEvals;

		// restore original vector
		for (unsigned int j=i; j<n; j += m) {
			// restore y vector
			m_yMod[j] = y[j];
			// restore ydot vector
			m_ydotMod[j] = ydot[j];
		} // for j
	} // for i

#ifdef DUMP_JACOBIAN_TEXT
	std::ofstream jacdump("jacobian_band.txt");
	m_jacobian->write(jacdump, nullptr, false, 15);
	jacdump.close();
	throw IBK::Exception("Done with test-dump of Jacobian", "[LESBand::setup]");
#endif
#ifdef DUMP_JACOBIAN_BINARY
	IBK::write_matrix_binary(*m_jacobian, "jacobian_band.bin");
	throw IBK::Exception("Done with test-dump of Jacobian", "[LESBand::setup]");
#endif

	// scale matrix with -gamma
	unsigned int dataSize = m_jacobian->data().size();
	double * data = m_jacobian->data().data();
	for (unsigned int i=0; i<dataSize; ++i)
		data[i] *= -gamma;
	// add identity matrix
	for (unsigned int i=0; i<n; ++i)
		(*m_jacobian)(i,i) += 1.0;

	// perform LU-factorization of the jacobian
	m_jacobian->lu();

	// update solver statistics
	++m_statNumJacEvals;
}


void LESBand::solve(double * rhs) {
	// only solve linear equation system explicitely if integrator implicit Euler is chosen
	if (dynamic_cast<IntegratorImplicitEuler*>(m_integrator) == nullptr)
		return;

	// backsolve with given lu factorisation
	m_jacobian->backsolve(rhs);
}


std::size_t LESBand::serializationSize() const {
	// determine which integrator we are using
	if (IntegratorSundialsCVODE* intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(m_integrator)) {
		// we are using the CVSpgmr solver
		std::size_t s = CVDlsSerializationSize(intCVODE->cvodeMem());
		return s;
	}
	else {
		 return NON_SUPPORTED_FUNCTION; // serialization not supported
	}
}


void LESBand::serialize(void* & dataPtr) const {
	// determine which integrator we are using
	if (IntegratorSundialsCVODE* intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(m_integrator)) {
		CVDlsSerialize(intCVODE->cvodeMem(), &dataPtr); // ignoring return value for now
	}
}


void LESBand::deserialize(void* & dataPtr) {
	// determine which integrator we are using
	if (IntegratorSundialsCVODE* intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(m_integrator)) {
		CVDlsDeserialize(intCVODE->cvodeMem(), &dataPtr); // ignoring return value for now
	}
}

} // namespace SOLFRA

