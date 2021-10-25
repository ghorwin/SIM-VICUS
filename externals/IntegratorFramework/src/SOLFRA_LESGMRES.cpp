#include "SOLFRA_LESGMRES.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "SOLFRA_IntegratorSundialsCVODE.h"
#include "SOLFRA_IntegratorImplicitEuler.h"
#include "SOLFRA_ModelInterface.h"
#include "SOLFRA_JacobianInterface.h"
#include "SOLFRA_PrecondInterface.h"

#include <cvode/cvode_spgmr.h>
#include <cvode/cvode_spils.h>
#include <cvode/cvode_serialization.h>

#include <IBKMK_BandMatrix.h>
#include <IBK_Exception.h>
#include <IBK_FormatString.h>
#include <IBK_math.h>
#include <IBK_messages.h>

//#define DEBUG_ERROR_TEST

namespace SOLFRA {

LESGMRES::LESGMRES() :
	m_hessian(nullptr),
	m_gamma(0.1),
	m_delta(0.1)
{
#ifdef DEBUG_EIGENVALUES
	m_eigenValueUpdateNeeded = false;
#endif
#ifdef DEBUG_ERROR_TEST
	m_linIterConvCoeff = 1.0;
#endif
}

LESGMRES::~LESGMRES() {
	delete m_hessian;
}

void LESGMRES::init(ModelInterface * model, IntegratorInterface * integrator, PrecondInterface * precond,
					JacobianInterface * jacobian)
{
	const char * const FUNC_ID = "[LESGMRES::init]";

	m_integrator = integrator;
	m_model      = model;
	m_precond    = precond;
	m_jacobian   = jacobian;


	// determine which preconditioner we are using
	if (precond == nullptr)
		IBK::IBK_Message(IBK::FormatString("No Preconditioner is set for LESGMRES linear equation solver."), IBK::MSG_WARNING, FUNC_ID, IBK::VL_ALL );

	// determine which integrator we are using
	if (IntegratorSundialsCVODE* intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(m_integrator)) {
		// create and register CVSpgmr with integrator
		int sundialsPrecType = PREC_NONE; // by default preconditioner is disabled
		// if we have a Jacobian matrix implementation, we need the preconditioner to be a dummy left-sided Preconditioner
		if (jacobian != nullptr)
			sundialsPrecType = PREC_LEFT;
		if (precond != nullptr) {
			switch (precond->preconditionerType()) {
				case PrecondInterface::Left		: sundialsPrecType = PREC_LEFT; break;
				case PrecondInterface::Right	: sundialsPrecType = PREC_RIGHT; break;
				case PrecondInterface::Both		: sundialsPrecType = PREC_BOTH; break;
				case PrecondInterface::None		:
					throw IBK::Exception(IBK::FormatString("Error initialising preconditioner for LESGMRES linear equation solver: "
														"Preconditioner attribute 'Left', 'Right' or 'Both is enforced."), FUNC_ID);
			}
		}
		int result = CVSpgmr(intCVODE->cvodeMem(), sundialsPrecType, (int)m_maxKrylovDim);
		if (result != CVSPILS_SUCCESS) {
			if (result == CVSPILS_MEM_FAIL)
				IBK::IBK_Message( "CVSpgmr memory initialization error (problem too large?)", IBK::MSG_ERROR, FUNC_ID);
			else if (result == CVSPILS_ILL_INPUT)
				IBK::IBK_Message( "CVSpgmr init error (wrong input?)", IBK::MSG_ERROR, FUNC_ID);
			else
				IBK::IBK_Message( "CVSpgmr init error", IBK::MSG_ERROR, FUNC_ID);
			throw IBK::Exception("Error initializing linear equation solver.", FUNC_ID);
		}

		// *** Set Linear Iteration Factor (EpsLin) ***
		if (m_linIterConvCoeff != 0.0) {
			IBK::IBK_Message( IBK::FormatString("Setting EpsLin (eplifac,linIterConvCoeff) to %1.\n").arg(m_linIterConvCoeff), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			CVSpilsSetEpsLin(intCVODE->cvodeMem(), m_linIterConvCoeff);
		}
	}
	else if(IntegratorImplicitEuler* intImplicitEuler = dynamic_cast<IntegratorImplicitEuler*>(m_integrator)) {
		(void)intImplicitEuler;
		// for now, only resize matrix and vectors when using implicit Euler (so that we are not wasting memory)
		unsigned int m = m_maxKrylovDim + 1;
		unsigned int n = model->n();
		// create hessian matrix of krylov space dimension: upper bandwidth = m - 1, lower bandwidth = 1
		// note: the last column of hessian is only a fill-in and always zero
		m_hessian = new IBKMK::BandMatrix(m, m - 1, 1, m);
		// Krylov subspace base base vectors are of lenght n (problem dimension) and maximum number m (krylov space dimension)
		m_V.resize(m,std::vector<double>(n,0));
		// Givens rotation stores QR-factors of hessian and is needed for residual calculation: maximum size 2*m
		m_givens.resize(2*m);
		// resize all model vectors
		m_W.resize(n,0.0);
		m_y.resize(n,0.0);
		m_weights.resize(n,0.0);
		m_yCorr.resize(n,0.0);
		m_yCorrTemp.resize(n,0.0);
		m_temp.resize(n,0.0);
		m_res.resize(n,0.0);
		m_tempKryl.resize(m,0.0);

#ifdef DEBUG_EIGENVALUES
		m_eigenValueEstH.resize(m-1,0.0);
#endif

		m_ydot.resize(n,0.0);
		if (m_linIterConvCoeff == 0.0)
			m_linIterConvCoeff = 0.1; // initialize to default value as in CVODE
		IBK::IBK_Message( IBK::FormatString("Setting LinIterConvCoeff to %1.\n").arg(m_linIterConvCoeff), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		m_delta = intImplicitEuler->m_nonLinConvCoeff * m_linIterConvCoeff * IBK::f_sqrt(double(m_model->n()) );
	}
	else {
		throw IBK::Exception(IBK::FormatString("Error initializing Sundials GMRES linear solver: "
			"Only 'IntegratorSundialsIDA' or 'IntegratorSundialsCVODE' "
			"are suitable integrators."), FUNC_ID);
	}
}


void LESGMRES::setup(const double * y, const double * ydot, const double * residuals, double gamma) {

	// only setup linear equation system explicitely if integrator implicit Euler is chosen
	IntegratorImplicitEuler* integratorImplicitEuler = dynamic_cast<IntegratorImplicitEuler*>(m_integrator);
	if ( integratorImplicitEuler == nullptr)
		return;

	// store all components for the calculation of J*v
	std::memcpy(&m_y[0],y,m_y.size()*sizeof(double));
	// reinterpret gamma = dt
	m_gamma = gamma;

	// calculate weights
	// use either the models error weights function
	if (m_model->hasErrorWeightsFunction()) {
		double *weights = &m_weights[0];
		SOLFRA::ModelInterface::CalculationResult res = m_model->calculateErrorWeights(&m_y[0], weights);
		(void)res; // to make compiler happy
		// ensure validity of the implementation
		IBK_ASSERT(res == SOLFRA::ModelInterface::CalculationSuccess);
	}
	// or equation-dependent absolute tolerances
	else if ( integratorImplicitEuler->m_absTolVec.empty() ) {
		for(unsigned int i = 0; i < m_weights.size(); ++i)
			m_weights[i] = 1.0/(std::fabs(m_y[i])*integratorImplicitEuler->m_relTol + integratorImplicitEuler->m_absTol );
	}
	// or vector of tolerances
	else {
		for(unsigned int i = 0; i < m_weights.size(); ++i)
			m_weights[i] = 1.0/(std::fabs(m_y[i])*integratorImplicitEuler->m_relTol + integratorImplicitEuler->m_absTolVec[i]);
	}


	std::memcpy(&m_ydot[0],ydot,m_ydot.size()*sizeof(double));

#ifdef DEBUG_EIGENVALUES
	// enforce an update of eigenvalues
	m_eigenValueUpdateNeeded      = true;
#endif

	double t = integratorImplicitEuler->t();

	// update Jacobian if dedicated implementation is provided and if CVODE signals
	// that Jacobian is outdated
	//bool jacGenerated = false;
	//int res;
	if (m_jacobian != nullptr) {
		// update df/dy
		// res = m_jacobian->setup(t, y, ydot, residuals, gamma);
		m_jacobian->setup(t, y, ydot, residuals, gamma);
		//jacGenerated = true;
	}
	bool jacUpdated = false;
	if (m_precond != nullptr) {
		// now call setup in precond
		//res = m_precond->setup(t, y, ydot, residuals, false, jacUpdated, gamma);
		m_precond->setup(t, y, ydot, residuals, false, jacUpdated, gamma);
		++m_statNumPrecEvals;
	}
}


void LESGMRES::solve(double * rhs) {
	// only setup linear equation system explicitely if integrator implicit Euler is chosen
	IntegratorImplicitEuler* integratorImplicitEuler = dynamic_cast<IntegratorImplicitEuler*>(m_integrator);
	if ( integratorImplicitEuler == nullptr)
		return;

	// translate preconsitioner type into boolean types
	bool precondLeft = false;
	bool precondRight = false;
	if (m_precond != nullptr) {
		precondLeft = (m_precond->preconditionerType() == PrecondInterface::Left ||
			m_precond->preconditionerType() == PrecondInterface::Both);
		precondRight = (m_precond->preconditionerType() == PrecondInterface::Right ||
			m_precond->preconditionerType() == PrecondInterface::Both);
	}

	bool converged = false;
	const unsigned int n = m_model->n();
	// get time point
	double time = 0;
	double resNorm = 0.0;
	std::fill(m_yCorr.begin(),m_yCorr.end(), 0.0);
	std::memcpy(&m_res[0],rhs,m_res.size()*sizeof(double));

#ifdef DEBUG_EIGENVALUES
	std::vector<double> eigenValueEstHTmp;
#endif
#ifdef DEBUG_ERROR_TEST
		double integratorResNorm = 0.0;
		double	dtRatioInv = 1.0;
		dtRatioInv     = integratorImplicitEuler->dtJacUpdate()/integratorImplicitEuler->dt();
#endif

	// set V to zero
	for( unsigned int i = 0; i < m_maxKrylovDim + 1; ++i) {
		std::fill(m_V[i].begin(), m_V[i].end(), 0.0);
	}

	// copy y and residuals
	// calculate first base: residual r0 = rhs - A*dy
	std::vector<double> &V0 = m_V[0];
	// start with V0 = rhs: the same as rhs - A*dy with dy = 0
	std::copy(m_res.begin(),m_res.end(),V0.begin());
	// copy V to temp
	std::copy(V0.begin(),V0.end(),m_temp.begin());
	//int result = ATimesVec(&V0[0], &V0[0], time, &m_y[0], &m_ydot[0], &m_residuals[0]);
	//for(unsigned int i = 0; i < n; ++i)
	//	V0[i] = rhs[i] - V0[i];

#ifdef DEBUG_ERROR_TEST
	integratorResNorm = 0.0;
	for(unsigned int i = 0; i < n; ++i)
	{
		double r_normalized = V0[i]* m_weights[i];
		integratorResNorm += r_normalized*r_normalized;
	}
	integratorResNorm = IBK::f_sqrt(integratorResNorm);
#endif

	// Apply left preconditioner to temp and store result in V0
	if(precondLeft) {
		double delta = 0.0;
		// what does delta mean?
		m_precond->solve(time,&m_y[0],&m_ydot[0],nullptr,&m_temp[0],&V0[0],
						m_gamma,delta,PREC_LEFT);
		// correct solver statistics
		++m_statNumPrecSolves;
	}
	// Apply scaling to V0
	for(unsigned int i = 0; i < n; ++i)
		V0[i] *= m_weights[i];

	// calculate residual norm
	resNorm = norm2(&V0[0], n);
	resNorm = IBK::f_sqrt(resNorm);
	double newResNorm      = resNorm;


#ifdef DEBUG_ERROR_TEST
	double normPrec = 0.0;
	// calculate maximum norm
	for(unsigned int i = 0; i < n; ++i)
		normPrec = std::max(normPrec, fabs(m_res[i]) * m_weights[i]);
	// calculate |P| = sqrt(n) * max(r)_i/|P^(-1)r|
	normPrec *= IBK::f_sqrt(n)/resNorm;

	double resNormTol		= normPrec * newResNorm;
	// if residual norm is already below macimum error than return
	if(resNormTol < m_delta)
		return;
#else
	// if residual norm is already below macimum error than return
	if(newResNorm < m_delta)
		return;
#endif

	// normalise V0 to vector of lenght 1
	for(unsigned int i = 0; i < n; ++i)
		V0[i] /= resNorm;
	// set hessian matrix to zero
	m_hessian->setZero();
	// factor for residual norm after QR-factorisation
	double rotationProduct = 1.0;

	unsigned int krylDim   = 0;
	// loop over krylov space dimension, starting with 1
	for(unsigned int l = 0; l < m_maxKrylovDim; ++l)
	{
		++krylDim;
		// retreive previous krylov base vector
		std::vector<double> &Vl = m_V[l];
		// copy V to temp
		std::memcpy(&m_temp[0],&Vl[0],n*sizeof(double) );

		// invert residual scaling
		for(unsigned int i = 0; i < n; ++i)
			m_temp[i] /= m_weights[i];

		// Apply right preconditioner: W = PRight^(-1) temp
		if(precondRight) {
			double delta = 0.0;
			// what does delta mean?
			m_precond->solve(time,&m_y[0],&m_ydot[0],nullptr,&m_temp[0],&m_W[0],
							m_gamma,delta,PREC_RIGHT);
			// correct solver statistics
			++m_statNumPrecSolves;
		}
		else {
			// W = temp
			std::memcpy(&m_W[0],&m_temp[0],n*sizeof(double) );
		}

		// calculate temp = A*w
		int result;
		result = ATimesVec(&m_W[0], &m_temp[0], time, &m_y[0], &m_ydot[0], nullptr);

		/// \todo error handling
		(void)result;

		// copy temp to W
		std::memcpy(&m_W[0],&m_temp[0],n*sizeof(double) );

		// Apply left preconditioner to temp and store result i W
		if(precondLeft) {
			double delta = 0.0;
			// what does delta mean?
			m_precond->solve(time,&m_y[0],&m_ydot[0],nullptr,&m_temp[0],&m_W[0],
							m_gamma,delta,PREC_LEFT);
			// correct solver statistics
			++m_statNumPrecSolves;
		}
		// Apply scaling to W
		for(unsigned int i = 0; i < n; ++i)
			m_W[i] *= m_weights[i];

		// perform Gram-Schmidt orthogonalisation with given W = A*vl, fill in l_th column of hessian
		// and the next orthogonal base vector V[l+1]
		ModifiedGramSchmidt(&m_W[0], l);

		// perform QR factorisation of the lth column of the hessian matrix and store
		// the new Q-matrix Ql inside givens rotation vector (only store s and c)
		QRfactorisation(l);

		// append rotation to the rotation product
		rotationProduct *= m_givens[2*l + 1];

#ifdef DEBUG_EIGENVALUES
		// store current eigenvalues: hessian diagonal after QR factorization holds preconditioned
		// matrix eigenvalues
		eigenValueEstHTmp.push_back((*m_hessian)(l,l));
#endif

		// correct solver statistics
		++m_statNumLinIters;

		// now calculate the new residual norm
		newResNorm		= resNorm * std::fabs(rotationProduct);

#ifdef DEBUG_ERROR_TEST
		// calulate error norm
		resNormTol		= normPrec * newResNorm;

		// break if the new residual norm falls below the tolerance delta
		if (resNormTol <= m_delta) {
			converged = true;
			break;
		}
#else
		// break if the new residual norm falls below the tolerance delta
		if (newResNorm <= m_delta) {
			converged = true;
			break;
		}
#endif
		// krylov dimension exceeds problem dimension
		if(krylDim == n)
			break;
	}

	// now calculate new solution guess y with
	// y = dy0 + V*temp and r = b - A*y != 0. This gives the solution r0 = A*V*temp or V^T*r0 = V^T*A*V*temp.
	// Given the Hessian = V^T*A*V (see Saad) and V^T * r_0 = |r_0| * V^T * V[0] =  |r_0| e_1 (orthogonality of the V-vectors)
	// the solution is y = dy0 + yCorr = dy0 + V*temp with temp = Hessian^(-^1) * |r_0| e_1.
	std::fill(m_temp.begin(),m_temp.end(), 0.0);
	std::fill(m_yCorrTemp.begin(),m_yCorrTemp.end(), 0.0);
	// set temp to y: temp = |r_0| e_1.
	m_temp[0] = resNorm;
	// calculate Hessian^(-1)*temp with given size of hessian (may be smaller than
	// maxKrylovDim if the orthogonalisation was successful
	QRsolve(krylDim, &m_temp[0]);

	// calculate yCorr = V*temp
	for( unsigned int j = 0; j < krylDim; ++j) 	{
		for(unsigned int i = 0; i < n; ++i)
			m_yCorrTemp[i] += m_V[j][i] * m_temp[j];
	}
	// invert residual scaling
	for(unsigned int i = 0; i < n; ++i)
		m_yCorrTemp[i] /= m_weights[i];

	// copy yCorr to temp
	std::memcpy(&m_temp[0],&m_yCorrTemp[0],n*sizeof(double) );

	// Apply right right precond.: yCorr := PRight^(-1) temp.
	if(precondRight) {
		double delta = 0.0;
		// what does delta mean?
		m_precond->solve(time,&m_y[0],&m_ydot[0],nullptr,&m_temp[0],&m_yCorrTemp[0],
						m_gamma,delta,PREC_RIGHT);
		// correct solver statistics
		++m_statNumPrecSolves;
	}

	// add new correction vector
	for(unsigned int i = 0; i < n; ++i)
		m_yCorr[i] += m_yCorrTemp[i];

	// validity check
	int result;
	result = ATimesVec(&m_yCorrTemp[0], &m_temp[0], time, &m_y[0], &m_ydot[0], nullptr);
	/// \todo error handling
	(void)result;

#ifdef DEBUG_EIGENVALUES
	// update eigenvalues
	if(m_eigenValueUpdateNeeded)
	{
		// store current eigenvalue estimates
		m_eigenValueEstH = eigenValueEstHTmp;
		m_eigenValueUpdateNeeded =false;
	}
#endif

#ifdef DEBUG_ERROR_TEST
	// check if converged
	integratorResNorm = 0.0;
	for(unsigned int i = 0; i < n; ++i) {
		m_res[i] -= m_temp[i];
		double r_normalized = m_res[i] * m_weights[i];
		integratorResNorm += r_normalized * r_normalized;
	}
	integratorResNorm = IBK::f_sqrt(integratorResNorm);
	// correct norm
	if (integratorResNorm > m_delta)
		converged = false;
	else
		converged = true;

#endif

	// initial solution was 0, so set rhs = yCorr
	std::memcpy(rhs,&m_yCorr[0],n*sizeof(double) );
	// correct solver statistics
	if(!converged)
		++m_statNumLinConvFails;
}


int LESGMRES::ATimesVec(const double * v, double * Av, double t, double * y, double * ydot, double* residuals) {

	(void) t;
	(void) residuals;

	// use jacTimesVec function of the preconditioner if possible
	if(m_jacobian != nullptr) {
		// ode system: ydot = fy
		if(m_model != nullptr) {
			// calculate J*v
			m_jacobian->jacTimesVec(v, Av);
			// now calculate  A*v = (I - gamma * J) *v
			for (unsigned int i=0; i<m_model->n(); ++i) {
				Av[i] *= -m_gamma;
				Av[i] += v[i];
			}
		}
		// dae system: residual = fy
		else
			m_jacobian->jacTimesVec(v, Av);
	}
	// calculate A*v via dF(y)/dv
	else {
		// retreive system size
		unsigned int n = m_model->n();
		// store current solution guess
		std::memcpy(&m_yMod[0], y, m_yMod.size()*sizeof(double));

		// calculate y-norm = 1/n * |y|
		double ynorm = 0.0;
		for (unsigned int i=0; i<n; ++i) {
			ynorm += y[i] * y[i];
		}
		ynorm = IBK::f_sqrt(ynorm);
		ynorm/= double(n);

		const double epsi = ynorm*m_relToleranceDQ + m_absToleranceDQ;

		// modify y[]
		for (unsigned int i=0; i<n; ++i) {
			m_yMod[i] += epsi * v[i];
		}
		// calculate ode derivative
		if(m_model != nullptr)
		{
			// calculate modified right hand side
			m_model->setY(&m_yMod[0]);
			// calculate modified right hand side of the model, and store f(t,y) in m_FMod
			m_model->ydot(&m_FMod[0]);
			// update statistics
			++m_statNumRhsEvals;

			for (unsigned int i=0; i<n; ++i) {
				// compute finite-differences column j in row i
				Av[i] = ( m_FMod[i] - ydot[i] )/epsi;
				Av[i] *= -m_gamma;
				Av[i] += v[i];
			}
		}

	}
	// correct solver statistics
	++m_statNumJacTimesEvals;
	return 0;
}


int LESGMRES::ClassicalGramSchmidt(const double *Avl, unsigned int l) {
	unsigned int n = m_model->n();

	// fill hessian matrix, lth column
	for (unsigned int i = 0; i <= l; i++) {
		// calculate dot product between V[i] and W[l]
		std::vector<double> &Vi = m_V[i];
		// h_il = <W,Vi> = <A*Vl,Vi>
		(*m_hessian)(i,l) = dotProduct(Avl, &Vi[0], n);
	}

	// copy W into VPlus
	std::vector<double> &VlPlus = m_V[l + 1];
	std::memcpy(&VlPlus[0], Avl, n*sizeof(double));
	double VNorm = norm2(Avl,n);
	VNorm = IBK::f_sqrt(VNorm);

	// perform orthogonal projection of W to krylov subspave of dimension l
	for (unsigned int i = 0; i <= l; i++) {
		// retreive vector V_i
		std::vector<double> &Vi = m_V[i];
		// correct W = A*Vl - sum h_il * V_i (i=0,..,l)
		for(unsigned int j = 0; j < n; ++j)
			VlPlus[j] -= (*m_hessian)(i,l) * Vi[j];
	}

	// Compute the norm of the new vector
	double new_VNorm = norm2(&VlPlus[0],n);
	new_VNorm = IBK::f_sqrt(new_VNorm);

	// check if new norm exceeds limit and perform reorthogonalisation if necessary
	if (1000.0 * new_VNorm < VNorm) {
		// use temp vector for storing results
		std::fill(m_tempKryl.begin(),m_tempKryl.end(),0.0);
		for ( unsigned int i = 0; i <=l ; ++i) {
			m_tempKryl[i] = dotProduct(&VlPlus[0], &m_V[i][0], n);
		}
		// correct W and hessian
		for (unsigned int i = 0; i <= l; ++i) {
			// retreive vector V_i
			std::vector<double> &Vi = m_V[i];
			// W_l := W_l - sum_i <W_l,V_i> * V_i
			for(unsigned int j = 0; j < n; ++j)
				VlPlus[j] -= m_tempKryl[i] * Vi[j];
			// h_il = h_il + <W_l,V_i>
			(*m_hessian)(i,l) += m_tempKryl[i];
		}
		// update norm of W
		new_VNorm = norm2(&VlPlus[0],n);
		new_VNorm = IBK::f_sqrt(new_VNorm);
	}
	// fill new norm into hessian matrix
	(*m_hessian)(l+1,l) = new_VNorm;
	// compute V[l+1] := V[l+1]/new_VNorm
	for(unsigned int j = 0; j < n; ++j)
		VlPlus[j] /= new_VNorm;

	return 0;
}


int LESGMRES::ModifiedGramSchmidt(const double *Avl, unsigned int l) {
	unsigned int n = m_model->n();

	// calculate norm of A*v[l]
	double VNorm = norm2(Avl,n);
	VNorm = IBK::f_sqrt(VNorm);
	// copy W into VPlus
	std::vector<double> &VlPlus = m_V[l + 1];
	std::memcpy(&VlPlus[0], Avl, n*sizeof(double));
	// fill hessian matrix, lth column
	for (unsigned int i = 0; i <= l; i++) {
	  // calculate dot product between V[i] and W[l]
	  std::vector<double> &Vi = m_V[i];
	  // h_il = <W,Vi> = <A*Vl,Vi>
	  (*m_hessian)(i,l) = dotProduct(Avl, &Vi[0], n);
		// perform orthogonal projection of VlPlus to krylov subspave of dimension l
	  for(unsigned int j = 0; j < n; ++j)
		VlPlus[j] -=  (*m_hessian)(i,l) * Vi[j];
	}

	// Compute the norm of the new vector
	double new_VNorm = norm2(&VlPlus[0],n);
	new_VNorm = IBK::f_sqrt(new_VNorm);

	// check if new norm exceeds limit and perform reorthogonalisation if necessary
	if (std::fabs(1000.0 * VNorm + new_VNorm - 1000.0 * VNorm) < 1e-23) {
		double new_Norm2 = 0.0;
		for ( unsigned int i = 0; i <=l ; ++i) {
			// retreive vector V_i
			std::vector<double> &Vi = m_V[i];
			const double vecProd = dotProduct(&VlPlus[0], &Vi[0], n);
			const double temp = 1000.0 * (*m_hessian)(i,l);
			// if vecProduct near to zero than continue
			if(std::fabs(temp + vecProd - temp) < 1e-23)
				continue;
			// correct hessian matrix
			(*m_hessian)(i,l) += vecProd;
			// correct Vl+1
			for(unsigned int j = 0; j < n; ++j)
				VlPlus[j] -= vecProd * Vi[j];
			// calculate vector norm
			new_Norm2 += IBK::f_sqrt((vecProd > 0) ? IBK::f_sqrt(vecProd) : 0);
		}
		// complete vector norm computation
		if (new_Norm2 > 1e-23) {
			const double vecProd = new_VNorm * new_VNorm - new_Norm2;
			new_VNorm = (vecProd > 0) ? IBK::f_sqrt(vecProd) : 0;
		}
	}
	// fill new norm into hessian matrix
	(*m_hessian)(l+1,l) = new_VNorm;
	// compute V[l+1] := V[l+1]/new_VNorm
	for(unsigned int j = 0; j < n; ++j)
		VlPlus[j] /= new_VNorm;

	return 0;
}


int LESGMRES::QRfactorisation(unsigned int l) {
	// if l > 0 we already calculated a QR-factorisation for the last column
	// of hessian and now we perform the factorisation Q0 * Q1 * ... * Ql-1 for the new column
	for (unsigned int k = 0; k < l; ++k) {
		// get the position inside given rotation vector
		unsigned int givensOffset = 2*k;
		// matrix Qk = (c,-s,s,h) is situated at position (k,k+1,k,k+1)
		// and rotates the elements in kth and (k+1)th row
		const double HesKL		= (*m_hessian)(k,l);
		const double HesKKPlusL = (*m_hessian)(k + 1,l);
		const double c = m_givens[givensOffset];
		const double s = m_givens[givensOffset + 1];
		(*m_hessian)(k,l)	  = c*HesKL - s*HesKKPlusL;
		(*m_hessian)(k + 1,l) = s*HesKL + c*HesKKPlusL;
	}

	double c = 0.0;
	double s = 0.0;
	// compute a new QR-factorisation for column  l
	// c = hl,l / sqrt(hll^2 + hl+1,l^2)
	// s = hl+1,l / sqrt(hll^2 + hl+1,l^2)
	const double HesLL	   = (*m_hessian)(l,l);
	const double HesLPlusL = (*m_hessian)(l + 1,l);
	// our new factorisation vector with lenght hessian(l+1,l) is zero:
	// without computing we know c = 1 and s = 0
	if (HesLPlusL == 0) {
		c = 1.0;
		s = 0.0;
	}
	else if (std::fabs(HesLPlusL) >= std::fabs(HesLL)) {
		const double temp3 = HesLL/HesLPlusL;
		s = - 1.0 / IBK::f_sqrt(1.0 + temp3 * temp3);
		c = - s * temp3;
	}
	else {
		const double temp3 = HesLPlusL/HesLL;
		c = 1.0 /IBK::f_sqrt(1.0 + temp3 * temp3);
		s = - c * temp3;
	}
	// transform hl,l
	(*m_hessian)(l,l) = c * HesLL - s * HesLPlusL;
	// todo: error check (hl,l == 0)
	unsigned int givensOffset = 2*l;
	// store c and s inside givens rotation vector
	m_givens[givensOffset] = c;
	m_givens[givensOffset + 1] = s;

	return 0;
}


int LESGMRES::QRsolve(unsigned int lmax, double* rhs) {

	// Compute Q0* Q1 * ...* Qlmax-1 *rhs
	for (unsigned int k=0; k < lmax; ++k) {
		unsigned int givensOffset = 2*k;
		const double c = m_givens[givensOffset];
		const double s = m_givens[givensOffset + 1];
		const double bK		= rhs[k];
		const double bKPlus = rhs[k + 1];
		rhs[k]	   = c*bK - s*bKPlus;
		rhs[k + 1] = s*bK + c*bKPlus;
	}

	// Now solve  R*x = Q*rhs
	for ( int k = lmax - 1; k >= 0; --k) {
		// error: singular matrix (Hessian(i,i) = 0)
		if ((*m_hessian)(k, k) == 0) {
			return 1;
		}
		rhs[k] /= (*m_hessian)(k, k);
		for (int i = 0; i < k; ++i)
			rhs[i] -= rhs[k] * (*m_hessian)(i,k);
	}
	// hessian(lmax,lmax-1) is expected to be near zero and is ignored
	// also ignore last column (lmax) of hessian
	return 0;
}


void LESGMRES::writeStatisticsHeader(const IBK::Path & logfilePath, bool doRestart) {
	LESInterfaceIterative::writeStatisticsHeader(logfilePath, doRestart);

#ifdef DEBUG_PRECOND
	// eigenvalue estimates are only available for implicit euler integrator
	if (IntegratorImplicitEuler* intImplEuler = dynamic_cast<IntegratorImplicitEuler*>(m_integrator)){
		std::stringstream strmEigVals;
		strmEigVals << logfilePath.str() << "/LES_gmres"
			 << std::setw(2) << std::setfill('0') << std::right << m_maxKrylovDim
			 << "_eigenvalues.txt";
		m_eigenvaluesFileStream = new std::ofstream(strmEigVals.str().c_str(), std::ios_base::app);
	}
#endif
}


void LESGMRES::writeStatistics(double t)  {
	LESInterfaceIterative::writeStatistics(t);

#ifdef DEBUG_PRECOND
	if (IntegratorImplicitEuler* intImplEuler = dynamic_cast<IntegratorImplicitEuler*>(m_integrator)){
		// write eigenvalue estimate
		std::ostream & eigenValOut = *m_eigenvaluesFileStream;
		// Time after last step was completed
		eigenValOut << std::setw(25) << std::right << "Time:";
		eigenValOut << std::fixed << std::setprecision(10) << std::setw(25) << std::right << t;
		eigenValOut << std::endl;
		eigenValOut << std::setw(25) << std::right << "Eigenvalues:";
		eigenValOut << std::endl;
		// now fill all eigenvalues for the current time step
		for(unsigned int i = 0; i < m_eigenValueEstH.size(); ++i)
		{
			eigenValOut << std::fixed << std::setprecision(10) << std::setw(25) << std::right << m_eigenValueEstH[i];
		}
		eigenValOut << std::endl;
		eigenValOut << std::endl;
	}
#endif
}


std::size_t LESGMRES::serializationSize() const {
	// determine which integrator we are using
	if (IntegratorSundialsCVODE* intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(m_integrator)) {
		// we are using the CVSpgmr solver
		std::size_t s = CVSpilsSerializationSize(intCVODE->cvodeMem());
		return s;
	}
	else {
		 return NON_SUPPORTED_FUNCTION;  // serialization not supported
	}
}


void LESGMRES::serialize(void* & dataPtr) const {
	// determine which integrator we are using
	if (IntegratorSundialsCVODE* intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(m_integrator)) {
		CVSpilsSerialize(intCVODE->cvodeMem(), &dataPtr); // ignoring return value for now
	}
}


void LESGMRES::deserialize(void* & dataPtr) {
	// determine which integrator we are using
	if (IntegratorSundialsCVODE* intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(m_integrator)) {
		CVSpilsDeserialize(intCVODE->cvodeMem(), &dataPtr); // ignoring return value for now
	}
}



double LESGMRES::dotProduct(const double *v, const double *w, unsigned int n) {
	double result = 0.0;
	for(unsigned int i = 0; i < n; ++i)
		result += v[i]*w[i];
	return result;
}

double LESGMRES::norm2(const double *v, unsigned int n) {
	double result = 0.0;
	for(unsigned int i = 0; i < n; ++i)
		result += v[i]*v[i];
	return result;
}

} // namespace SOLFRA

