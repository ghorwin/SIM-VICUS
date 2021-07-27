#include "SOLFRA_LESBiCGStabP2.h"

#include <iomanip>
#include <iostream>
#include <fstream>

#include "SOLFRA_IntegratorSundialsCVODE.h"
#include "SOLFRA_IntegratorImplicitEuler.h"
#include "SOLFRA_ModelInterface.h"

#include <sundials/sundials_types.h>
#include <sundials/sundials_math.h>
#include <nvector/nvector_serial.h>
#include <cvode/cvode.h>
#include <cvode/cvode_spbcgs.h>
#include <cvode/cvode_spils.h>
#include <ida/ida_spbcgs.h>
#include <ida/ida_spils.h>

#include <IBK_Exception.h>
#include <IBK_FormatString.h>
#include <IBK_messages.h>

#include <IBKMKC_vector_operations.h>

namespace SOLFRA {



LESBiCGStabP2::LESBiCGStabP2()
{
}

void LESBiCGStabP2::init(ModelInterface *  model, IntegratorInterface * integrator, PrecondInterface * precond, JacobianInterface *jacobian) {
	const char * const FUNC_ID = "[LESBiCGStabP2::init]";

	m_integrator	= integrator;
	m_jacobian		= jacobian;
	m_model			= model;
	m_precond		= precond;

	// determine which preconditioner we are using
	if (precond == NULL)
		IBK::IBK_Message(IBK::FormatString("No Preconditioner is set for LESBiCGStab linear equation solver."), IBK::MSG_WARNING, FUNC_ID, IBK::VL_ALL );

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
					throw IBK::Exception(IBK::FormatString("Error initialising preconditioner for LESBiCGStab linear equation solver: "
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
		m_rRoof.resize(problemSize);
		m_r0.resize(problemSize);
		m_r.resize(problemSize);
		m_s.resize(problemSize);
		m_t.resize(problemSize);
		m_u.resize(problemSize);
		m_v.resize(problemSize);
		m_w.resize(problemSize);
		m_wRoof.resize(problemSize);

		return;
	}

	// for now we only support SundialsCVODE or SundialsIDA
	throw IBK::Exception(IBK::FormatString("Error initializing Sundials BCGStab linear solver: "
		"Only 'IntegratorSundialsIDA' or 'IntegratorSundialsCVODE' "
		"are suitable integrators."), FUNC_ID);
}


void LESBiCGStabP2::setup(const double * y, const double * ydot, const double * residuals, double gamma) {
	(void)y;
	(void)ydot;
	(void)residuals;
	(void)gamma;



//	if ( IntegratorImplicitEuler* intImplicitEuler = dynamic_cast<IntegratorImplicitEuler*>(m_integrator) ) {

//		// there is no weight handling yet

//		// we need to create jacobian matrix here and
//		if (m_jacobian != NULL) {
//			// update df/dy
//			m_jacobian->setup(t, y, ydot, residuals, gamma);
//		}

//	}
}


void LESBiCGStabP2::solve( double * b ) {


	/// \todo implement this
	(void) b;


	// this will be an parallel iteration unrooled BiCGStab

	// supposed to work only with integrator implicit Euler
	IntegratorImplicitEuler* integratorImplicitEuler = dynamic_cast<IntegratorImplicitEuler*>(m_integrator);
	if ( integratorImplicitEuler == NULL)
		return;
//	/*! */
//	double	alpha;
//	/*! */
//	double	beta;
//	/*! */
//	double	gamma;
//	/*! */
//	double	delta;
//	/*! */
//	double	eta;
//	/*! */
//	double	theta;
//	/*! */
//	double	kappa;
//	/*! */
//	double	phi;
//	/*! */
//	double	pi;
//	/*! */
//	double	rho0;
//	/*! previous iteration value of rho */
//	double	rho1;
//	/*! */
//	double	sigma;
//	/*! previous iteration value of sigma */
//	double	sigma1;
//	/*! */
//	double	tau;
	/*! */
//	double	omega1;
//	double	omega2;

//	/// initial guess for x is 0 so, we need an vector operation here as well
//#if defined(_OPENMP)
//	IBKMK::vector_fill(m_x.size(), &m_x[0], 0.0 );
//#else
//	std::fill(m_x.begin(),m_x.end(), 0.0);
//#endif


//	/// clear all helper (MPI must maintain borders here!)
//#if defined(_OPENMP)
//	IBKMK::vector_fill(m_p.size(), &m_p[0], 0.0 );
//	IBKMK::vector_fill(m_v.size(), &m_v[0], 0.0 );
//	IBKMK::vector_fill(m_w.size(), &m_w[0], 0.0 );
//#else
//	std::fill(m_p.begin(),m_p.end(), 0.0);
//	std::fill(m_v.begin(),m_v.end(), 0.0);
//	std::fill(m_w.begin(),m_w.end(), 0.0);
//#endif

//	// compute r0 = b - Ax0 -> x0 -> 0, we depend on an initial guess here,... other possibilities ?
//	IBKMK::vector_copy(m_r0.size(), b, &m_r0[0] );

//	// init r's
//	IBKMK::vector_copy(m_r0.size(), m_r0, &m_r[0] );
//	IBKMK::vector_copy(m_r0.size(), m_r0, &m_rRoof[0] );

//	// compute s with r0
//	m_jacobian->jacTimesVec( &m_r0[0], &m_s[0] );
//	IBKMK::vector_copy(m_s.size(), m_s, &m_t[0] );
//	m_jacobian->jacTimesVec( &m_t[0], &m_u[0] );
//	m_jacobian->jacTimesVec( &m_w[0], &m_X[0] );



//	/// init greeks
//	rho1	= 1.0;
//	alpha	= 1.0;
//	omega1	= 1.0;
//	omega2	= 1.0;

//	kappa = 0.;
//	theta = 0.;
//	eta = 0.;
//	phi = 0.;

//#if defined(_OPENMP)
//#pragma omp for reduction(+:kappa,theta,eta) nowait
//#endif
//	for (unsigned int i = 0; i < problemSize; ++i){
//		kappa	+= m_r[i]*m_rRoof[i];
//		theta	+= m_s[i]*m_rRoof[i];
//		eta		+= m_t[i]*m_rRoof[i];
//		phi		+= m_u[i]*m_rRoof[i];
//		pi		+= m_w[i]*m_rRoof[i];
//	}

//	/// main iteration loop
//	for ( unsigned int n = 0; n < m_maxKrylovDim; n +=2 ){

//		rho0 = -1 * omega2 * rho1;

//		// even BiCGStep
//		rho1 = kappa - omega1 * theta - omega2 * eta;
//		beta = alpha * rho1 / rho0;
//		rho0 = rho1;

//#if defined(_OPENMP)
//#pragma omp for nowait
//#endif
//		for (unsigned int i = 0; i < problemSize; ++i){
//			m_p[i] = m_r[i] - beta * (m_p[i] - omega1*m_v[i]-omega2*m_w[i]);
//			m_v[i] = m_s[i] + omega1*m_t[i] - omega2*m_u[i] - beta * (gamma - omega1*m_w[i]-omega2*m_X[i]);
//		}

//		m_jacobian->jacTimesVec( &m_v[0], &m_wRoof[0] );

//		gamma = theta - omega1*eta-omega2*phi-beta*( gamma - omega1*h - omega2*k);
//		alpha = rho0 / gamma;

//#if defined(_OPENMP)
//#pragma omp for nowait
//#endif
////		for (unsigned int i = 0; i < problemSize; ++i){
////			m_r[i] =

////		}


		// odd BiCGStep


		// GMRES (2) part


//	} // for ( unsigned int n = 0; n < m_maxKrylovDim; n +=2 ) // loop until convergence

//	// correct solver statistics
//	++m_statNumLinConvFails;

}

} // namespace SOLFRA

