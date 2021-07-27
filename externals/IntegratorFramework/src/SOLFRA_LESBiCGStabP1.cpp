#include "SOLFRA_LESBiCGStabP1.h"

#include <cstring>
#include <cmath>

#include "SOLFRA_IntegratorSundialsCVODE.h"
#include "SOLFRA_IntegratorImplicitEuler.h"
#include "SOLFRA_JacobianInterface.h"

#include <cvode/cvode_spbcgs.h>
#include <cvode/cvode_spils.h>
#include <ida/ida_spbcgs.h>
#include <ida/ida_spils.h>

#include <IBK_Exception.h>
#include <IBKMK_SparseMatrix.h>
#include <IBK_FormatString.h>
#include <IBK_messages.h>
#include <IBK_math.h>


#include <IBKMKC_vector_operations.h>

namespace SOLFRA {

LESBiCGStabP1::LESBiCGStabP1():
	m_deltaNorm(0.5)
{
}


void LESBiCGStabP1::init(ModelInterface *  model, IntegratorInterface * integrator, PrecondInterface * precond, JacobianInterface *jacobian) {
	const char * const FUNC_ID = "[LESBiCGStabP1::init]";

	m_integrator	= integrator;
	m_jacobian		= jacobian;
	m_model			= model;
	m_precond		= precond;

	// determine which preconditioner we are using
	if (precond == NULL)
		IBK::IBK_Message(IBK::FormatString("No Preconditioner is set for LESBiCGStabP1 linear equation solver."), IBK::MSG_WARNING, FUNC_ID, IBK::VL_ALL );

	// determine which integrator we are using
	if (IntegratorSundialsCVODE* intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(m_integrator)) {
		// create and register CVSpbcg with integrator
		int sundialsPrecType = PREC_NONE;
		if (precond == NULL) {
			if (jacobian != NULL)
				sundialsPrecType = PREC_LEFT; // dummy-preconditioner for supporting J*v implementation
		}
		else {
			switch (precond->preconditionerType()) {
				case PrecondInterface::Left		: sundialsPrecType = PREC_LEFT; break;
				case PrecondInterface::Right	: sundialsPrecType = PREC_RIGHT; break;
				case PrecondInterface::Both		: sundialsPrecType = PREC_BOTH; break;
				case PrecondInterface::None		:
					throw IBK::Exception(IBK::FormatString("Error initialising preconditioner for LESBiCGStabP1 linear equation solver: "
														"Preconditioner attribute 'Left', 'Right' or 'Both is enforced."), FUNC_ID);
				default							:  break;
			}
		}
		int result = CVSpbcg(intCVODE->cvodeMem(), sundialsPrecType, m_maxKrylovDim);
		if (result != CVSPILS_SUCCESS) {
			if (result == CVSPILS_MEM_FAIL)
				IBK::IBK_Message( "CVSpbcg memory initialization error (problem too large?)", IBK::MSG_ERROR, FUNC_ID);
			else if (result == CVSPILS_ILL_INPUT)
				IBK::IBK_Message( "CVSpbcg init error (wrong input?)", IBK::MSG_ERROR, FUNC_ID);
			else
				IBK::IBK_Message( "CVSpbcg init error", IBK::MSG_ERROR, FUNC_ID);
			throw IBK::Exception("Error initializing linear equation solver.", FUNC_ID);
		}

		// *** Set Linear Iteration Factor (EpsLin) ***
		if (m_linIterConvCoeff != 0) {
			IBK::IBK_Message( IBK::FormatString("Setting EpsLin (eplifac,linIterConvCoeff) to %1.\n").arg(m_linIterConvCoeff), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			CVSpilsSetEpsLin(intCVODE->cvodeMem(), m_linIterConvCoeff);
		}

		return;
	}

	else if ( IntegratorImplicitEuler* intImplicitEuler = dynamic_cast<IntegratorImplicitEuler*>(m_integrator) ) {

		(void)intImplicitEuler;

		// cache model size
		unsigned int problemSize = model->n();

		// resize all latin vectors
		m_f.resize(problemSize);
		m_r.resize(problemSize);
		m_s.resize(problemSize);
		m_t.resize(problemSize);
		m_q.resize(problemSize);
		m_u.resize(problemSize);
		m_v.resize(problemSize);
		m_x.resize(problemSize);
		m_z.resize(problemSize);

		return;
		return;
	}

	// for now we only support SundialsCVODE or SundialsIDA
	throw IBK::Exception(IBK::FormatString("Error initializing Sundials BCGStab linear solver: "
		"Only 'IntegratorSundialsIDA' or 'IntegratorSundialsCVODE' "
		"are suitable integrators."), FUNC_ID);
}


void LESBiCGStabP1::setup(const double * y, const double * ydot, const double * residuals, double gamma) {

	/// \todo write correct init code
	if ( IntegratorImplicitEuler* integratorImplicitEuler = dynamic_cast<IntegratorImplicitEuler*>(m_integrator) ) {

		// there is no weight handling yet
		// calculate weights
		for(unsigned int i = 0; i < m_weights.size(); ++i)
			m_weights[i] = 1.0/(std::fabs(y[i])*integratorImplicitEuler->m_relTol + integratorImplicitEuler->m_absTol);


		// we need to create jacobian matrix here and
		double t = integratorImplicitEuler->t();
		if (m_jacobian != NULL) {
			// update df/dy
			m_jacobian->setup(t, y, ydot, residuals, gamma);
		}

		// now call setup in precond
		bool jacUpdated = false;
		if (m_precond != NULL) {
			m_precond->setup(t, y, ydot, residuals, false, jacUpdated, gamma);
			++m_statNumPrecEvals;
		}


	}
}


void LESBiCGStabP1::solve( double * b ) {

	// this is a parallel algorithm, not transposed matrix multy free
	/// \todo preconditioner and weights handling

	// supposed to work only with integrator implicit Euler
	IntegratorImplicitEuler* integratorImplicitEuler = dynamic_cast<IntegratorImplicitEuler*>(m_integrator);
	if ( integratorImplicitEuler == NULL)
		return;

	/* all greeks are scalar. */

	/*! */
	double	alpha;
	/*! */
	double	beta;
	/*! */
	double	gamma;
	/*! */
	double	delta;
	/*! */
	double	eta;
	/*! */
	double	theta;
	/*! */
	double	kappa;
	/*! */
	double	phi;
	/*! */
	double	pi;
	/*! */
	double	rho;
	/*! previous iteration value of rho */
	double	rho1;
	/*! */
	double	sigma;
	/*! previous iteration value of sigma */
	double	sigma1;
	/*! */
	double	tau;
	/*! */
	double	omega;


	double norm2 = 0.0;

#if defined(_OPENMP)
#pragma omp parallel
{
#endif


	/// initial guess for x is 0 so, we need an vector operation here as well
#if defined(_OPENMP)
	IBKMK::vector_fill(m_x.size(), &m_x[0], 0.0 );
#else
	std::fill(m_x.begin(),m_x.end(), 0.0);
#endif

	/// that's why our initial bicgstab residual equals solution (which must not be zero)
	IBKMK::vector_copy((unsigned int)m_r.size(), b, &m_r[0] );
	// std::memcpy(&m_r[0],b,m_r.size()*sizeof(double));

	/// that why our initial bicgstab residual equals solution (which must not be zero)
	IBKMK::vector_copy((unsigned int)m_r0.size(), b, &m_r0[0] );
	// std::memcpy(&m_r0[0],b,m_r0.size()*sizeof(double));


	/// initial calculation of u \todo think about parallel code here
	m_jacobian->jacTimesVec( &m_r[0], &m_u[0]);

	/// initial calculation of reuse vector f = A^T*r_0; this is a bit against memory oder but only executed once per solve
	/// \todo think about parallel code here and howdo we access such an function through an jacobian interface
//	m_jacobian->jacTransTimesVec( &m_r[0], &m_f[0]);

	/// clear all helper (MPI must maintain borders here!)
#if defined(_OPENMP)
	IBKMK::vector_fill(m_q.size(), &m_q[0], 0.0 );
	IBKMK::vector_fill(m_v.size(), &m_v[0], 0.0 );
	IBKMK::vector_fill(m_z.size(), &m_z[0], 0.0 );
#else
	std::fill(m_q.begin(),m_q.end(), 0.0);
	std::fill(m_v.begin(),m_v.end(), 0.0);
	std::fill(m_z.begin(),m_z.end(), 0.0);
#endif


	/// init greeks

	/// please keep in mind that iteration state of greeks depend on position in code not name!
	sigma1	= 0.0;
	pi		= 0.0;
	phi		= 0.0;
	tau		= 0.0;

	rho1	= 1.0;
	alpha	= 1.0;
	omega	= 1.0;

	/// this should be an single core instruction, inner product from some optimized code (u_0,r_0)_2
	/// since we are in R3 we just have a normal skalar product here
	sigma	= 0.0;
	int problemSize = m_model->n();

#if defined(_OPENMP)
#pragma omp for reduction(+:sigma) nowait
#endif
	for (int i = 0; i < problemSize; ++i){
		sigma += m_u[i]*m_r[i];
	}

	/// main iteration loop
	for ( int n = 1; n < (int)m_maxKrylovDim; ++n ){

		// please mind phi, omega, alpha, pi state at n-1, sigma at n-2
		rho = phi - omega * sigma1 + omega * alpha * pi;

		// alpha, omega, rho1 at n-1,
		delta	= rho * alpha / rho1;
		beta	= delta / omega;

		// sigma, tau, pi at n-1
		tau	= sigma + beta * tau - delta * pi;

		alpha = rho / tau;

		// some vector operation could be called here -> fully indipendent MPI -> rank oriented memory access possible
		// please not m_u, m_q are in state n-1
#if defined(_OPENMP)
#pragma omp for nowait
#endif
		for (int i = 0; i < problemSize; ++i){
			m_v[i] = m_u[i] + beta*m_v[i] - delta*m_q[i];
		}

		// independent matrix vector operation, MPI sequentially behind previous operation
		m_jacobian->jacTimesVec( &m_v[0], &m_q[0] );

		// Following two operations are fully independent from each other, thus we need fast MPI vector updates here
		// please note m_r, m_u are at n-1
#if defined(_OPENMP)
#pragma omp for nowait
#endif
		for (int i = 0; i < problemSize; ++i){
			m_t[i] = m_r[i] - alpha * m_v[i];
			m_s[i] = m_u[i] - alpha * m_q[i];
		}

		// MPI locally independent inner products, gathering results can be shadowed by following instruction
		pi		= 0.0;
		phi		= 0.0;
		kappa	= 0.0;
		gamma	= 0.0;
		eta		= 0.0;
		theta	= 0.0;
#if defined(_OPENMP)
#pragma omp for reduction(+:phi,pi,gamma,eta,theta,kappa) nowait
#endif
		for (int i = 0; i < problemSize; ++i){
			phi		+= m_r0[i]*m_s[i];
			pi		+= m_r0[i]*m_q[i];
			gamma	+= m_f[i]*m_s[i];
			eta		+= m_f[i]*m_t[i];
			theta	+= m_s[i]*m_t[i];
			kappa	+= m_t[i]*m_t[i];
		}

		// start nonblocking MPI_ALL_REDUCE_SUM here
		// then start parallel calculation of m_z
		// m_r, m_v, and m_r are at state n-1
		double alphaTimesDelta = alpha * delta;
#if defined(_OPENMP)
#pragma omp for nowait
#endif
		for (int i = 0; i < problemSize; ++i){
			m_z[i] = alpha * m_r[i] + beta * m_z[i] - alphaTimesDelta * m_v[i];
		}

		// hopefully we are done with MPI_ALL_REDUCE, we sync here now we can update omega and sigma to state n
		omega = theta / kappa;
		sigma1 = sigma;
		rho1 = rho;
		sigma = gamma - omega * eta;

		// update iterated residual vector r and corresponding vector x
		// next two vector updates are independent again, m_s, omega is coming from local cache, m_x partlly as well
#if defined(_OPENMP)
#pragma omp for nowait
#endif
		for (int i = 0; i < problemSize; ++i){
			m_r[i] = m_s[i] - omega * m_t[i];
			m_x[i] = m_x[i] + m_z[i] + omega * m_s[i];
		}

		// if m_x is accurate enough stop here
		// can we use a distributed delta norm upper bound here, or
		// non blocking MPI_ALL_REDUCE, start with next calculation iteration independently  ???
		norm2 = 0.0;
#if defined(_OPENMP)
#pragma omp for reduction(+:norm2) nowait
#endif
		for (int i = 0; i < problemSize; ++i){
			norm2 += m_x[0] * m_x[0];
		}
		norm2 = IBK::f_sqrt(norm2);

		if ( norm2 < m_deltaNorm ){

			/// return result in b, can be ommitted when calculation is executed in b, but pollution of b is not discussed yet
			IBKMK::vector_copy((unsigned int)m_x.size(),&m_x[0],b);

			/// \todo think about howto abort here if norm is fullfilled
			/// return;
		}

		// if not accurate enough update m_u for next iteration
		m_jacobian->jacTimesVec( &m_r[0], &m_u[0] );

	} // end of iteration loop



#if defined(_OPENMP)
} // parallel section
#endif

	// What happens when for ( unsigned int n = 1; n < m_maxKrylovDim; ++n ) is not enough
	// correct solver statistics
	++m_statNumLinConvFails;

}

} // namespace SOLFRA

