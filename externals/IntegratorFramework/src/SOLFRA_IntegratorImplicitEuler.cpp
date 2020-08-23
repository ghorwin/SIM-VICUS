/*	Solver Control Framework
	Copyright (C) 2010  Andreas Nicolai <andreas.nicolai -[at]- tu-dresden.de>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "SOLFRA_IntegratorImplicitEuler.h"

#include <iostream>
#include <iomanip>
#include <cmath>
#include <fstream>
#include <algorithm>

#include <IBK_assert.h>
#include <IBK_Time.h>
#include <IBK_messages.h>

#include <IBKMKC_vector_operations.h>
#include <IBK_openMP.h>

#include <sundials/sundials_timer.h>

#include "SOLFRA_LESInterface.h"
#include "SOLFRA_PrecondInterface.h"
#include "SOLFRA_JacobianInterface.h"

//#define SOLVER_STEP_STATS


namespace SOLFRA {

inline double * DOUBLE_PTR(std::vector<double> & vec) { return &vec[0]; }
inline const double * DOUBLE_PTR(const std::vector<double> & vec) { return &vec[0]; }

/*! Error norm-based scale factor used in time step adjustment. */
const double ERROR_NORM_SAFETY = 6;

IntegratorImplicitEuler::IntegratorImplicitEuler() :
	m_maxNonLinIters(3),
	m_NLResidualTolerance(1e-5),
	m_nonLinConvCoeff(0.1),
	m_modifiedNewtonStrategy(MN_ONCE_AT_STEP_START),
	m_dtMax(3600),
	m_statNumSteps(0),
	m_statNumRHSEvals(0),
	m_statNumIters(0),
	m_statNumConvFails(0),
	m_statNumErrFails(0),
	m_statNumConvDiverging(0),
	m_statNumJacEvals(0),
	m_model(nullptr),
	m_lesSolver(nullptr),
	m_preconditioner(nullptr),
	m_dt(0),
	m_statsFileStream(nullptr)
{
}


IntegratorImplicitEuler::~IntegratorImplicitEuler() {
	delete m_statsFileStream;
}


void IntegratorImplicitEuler::init(	ModelInterface * model,
									double t0,
									const double *y0,
									LESInterface *lesSolver,
									PrecondInterface *precond,
									JacobianInterface *jacobian)
{
	const char * const FUNC_ID = "[IntegratorImplicitEuler::init]";

	IBK::IBK_Message( "Initializing Implicit Euler integrator\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	m_model = model;
	IBK_ASSERT(m_model != nullptr);

	m_lesSolver = lesSolver;
	if (m_lesSolver == nullptr)
		throw IBK::Exception("Expected valid pointer to LES-solver object.", FUNC_ID);

	m_preconditioner = precond;

	m_t = t0;
	m_dt = model->dt0();

	m_dtNext = m_dtJacUpdate = m_dt;

	// resize vectors
	m_n=model->n();
	m_y.resize(m_n);
	m_ydot.resize(m_n);
	m_ydotn.resize(m_n);
	m_yn.resize(m_n);
	m_yPred.resize(m_n);
	m_yout.resize(m_n);
	m_residuals.resize(m_n);
	m_deltaY.resize(m_n);
	m_weights.resize(m_n);
	m_errors.resize(m_n);

	// transfer initial conditions
	IBKMK::vector_copy(m_n, y0, DOUBLE_PTR(m_y));

	// initialize state of model to initial conditions
	ModelInterface::CalculationResult res = m_model->setTime(m_t);
	if (res != ModelInterface::CalculationSuccess)
		throw IBK::Exception("Error initializing the model in setTime(): failed setting the start time.", FUNC_ID);

	res = m_model->setY(DOUBLE_PTR(m_y));
	if (res != ModelInterface::CalculationSuccess)
		throw IBK::Exception("Error initializing the model in setY(): failed setting the initial states.", FUNC_ID);

	// retrieve initial derivatives
	res = m_model->ydot(DOUBLE_PTR(m_ydot)); ++m_statNumRHSEvals;
	if (res != ModelInterface::CalculationSuccess)
		throw IBK::Exception("Error initializing the model in ydot(): failed setting the initial states.", FUNC_ID);

	// check for size of absTolVec
	if (!m_absTolVec.empty() && m_absTolVec.size() != m_n)
		throw IBK::Exception("Invalid size of absTolVec.", FUNC_ID);

	IBK::IBK_Message( IBK::FormatString("Setting NonlinConvCoef to %1.\n").arg(m_nonLinConvCoeff), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("Setting MaxNonLinIters to %1.\n").arg(m_maxNonLinIters), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	// *** Initialize Linear Equation Solver ***
	lesSolver->init(m_model, this, precond, jacobian);

	// *** Initialize Jacobian matrix ***
	if (jacobian != nullptr)
		jacobian->init(m_model);

	// *** Initialize Preconditioner ***
	if (precond != nullptr)
		precond->init(m_model, this, jacobian);

	// reset counters
	m_nItersSinceJacUpdate = 0;
	m_nStepsSinceJacUpdate = 0;
	m_newtonConvergenceError = false;

#ifdef SUNDIALS_USE_INSTRUMENTATION
	for (int i=0; i<SUNDIALS_TIMER_COUNT; ++i)
		TimerSumReset(i);
#endif // SUNDIALS_USE_INSTRUMENTATION
}


IntegratorInterface::StepResultType IntegratorImplicitEuler::step() {
	const char * const FUNC_ID = "[IntegratorImplicitEuler::step]";
	// input state:		m_t contains tn
	//					m_dtNext contains proposed time step for next integration step
	//					m_y contains yn
	//					m_ydot contains ydotn

	//					model state is undefined
	//					(can be m_tn, m_yn or any other value when outputs were written previously)

	// backup current solution in xxxn variables
	m_yn.swap(m_y);			// m_yn contains yn
	m_ydotn.swap(m_ydot);	// m_ydotn contains dydtn

	// set maximum scale factor for time step enlargement
	m_dtScaleMax = 10;

	// update the model's state to the previous solution, time state is set below
	ModelInterface::CalculationResult res;

	// *** time step reduction loop ***
	for (;;) {
		// reset iteration counter
		m_nIterations = 0;

		// move new time step suggestion to m_dt
		m_dt = m_dtNext;

		// compute new time point that we solve our equation system for
		double tnew = m_t + m_dt;

		// restrict time value to stop time
		// safeguard against bad m_stopTime value
		IBK_ASSERT(m_stopTime == 0 || m_stopTime > m_t);
		if (m_stopTime != 0 && tnew > m_stopTime)
			tnew = m_stopTime;

		// update the model's time-dependent components to this new time point
		res = m_model->setTime(tnew);
		handleErrors(res);

		// *** Prediction ***

		// compute estimate y_{m=0} of new solution using slopes at last time level
		// m_ydotn holds the derivativ of y_n, i.e. the functional evaluation of f(t_n, y_n)
		// m_y[i] = m_yn[i] + m_ydotn[i]*m_dt;
		IBKMK::vector_linear_sum(m_n, IBKMK_ONE, DOUBLE_PTR(m_yn), m_dt, DOUBLE_PTR(m_ydotn), DOUBLE_PTR(m_y));
		IBKMK::vector_copy(m_n, DOUBLE_PTR(m_y), DOUBLE_PTR(m_yPred));

//#define USE_CONSTANT_EXTRAPOLATION
#ifdef USE_CONSTANT_EXTRAPOLATION

		// we use the last yn values as starting values for the Newton iteration instead of
		// the predicted (explicit Euler) solution
		IBK::vector_copy(m_n, DOUBLE_PTR(m_yn), DOUBLE_PTR(m_y));

#else //  USE_CONSTANT_EXTRAPOLATION

		SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_FEVAL,
			evaluateModel(); // calculates m_ydot, m_residuals and m_residualNorm
		);

		// write iteration statistics
		writeIterationStats(0, 0, 0);

#endif //  USE_CONSTANT_EXTRAPOLATION

		// check if residual norm is too large which indicates that extrapolation may not be suitable
		const double RES_NORM_LIMIT = 300000; // this is a GUESS!!!!
		if (m_residualNorm > RES_NORM_LIMIT) {
			IBK::IBK_Message( IBK::FormatString("Reducing time step, residual of extrapolation was  %1.").arg(m_residualNorm), IBK::MSG_WARNING,
				FUNC_ID, IBK::VL_DETAILED);
			decreaseTimeStepNonLinearFailure();
			continue;
		}

		// try to take a non-linear step, from m_t using step size m_dt
		StepResult result = tryStep();

		if (result == Success) {
			bool errTestSuccess = errorTest();
			if (errTestSuccess) {
				adjustTimeStep(); // increase time step based on error test
				break; // break loop because we passed the error test
			}
		}
		else if (result == TryAgain) {
			// retry step with same time step size but énforce Jacobian update
			m_newtonConvergenceError = true;
		}
		else {
			decreaseTimeStepNonLinearFailure();
		}

	} // *** time step reduction loop ***


#ifdef SOLVER_STEP_STATS2
	std::cout
		<< std::setw(12) << std::right << m_t/3600 << " "
		<< std::setw(10) << std::right << m_dt << " "
		<< std::setw(6) << std::right << m_statNumSteps
		<< std::setw(6) << std::right << m_statNumIters
		<< std::setw(6) << std::right << m_statNumRHSEvals
		<< std::setw(6) << std::right << m_statNumJacEvals
		<< std::setw(6) << std::right << m_statNumConvFails
		<< std::setw(6) << std::right << m_statNumConvDiverging
		<< std::endl;
#endif // SOLVER_STEP_STATS

	// advance solution in time
	m_t += m_dt;
	// restrict time value to stop time
	if(m_stopTime != 0 && m_t > m_stopTime)
		m_t = m_stopTime;

	// m_y and m_ydot hold solution and time derivatives at time level m_t
	// model state is set to m_t, and m_y

	++m_statNumSteps;
	return IntegratorInterface::StepSuccess;
}


const double * IntegratorImplicitEuler::yOut(double t_out) const {
	if (t_out == m_t)
		return &m_y[0];
	// interpolate between last step and current step
	double t = std::min(t_out, m_t); // we don't allow extrapolations
	double alpha = (m_t-t)/m_dt;
	double beta = 1-alpha;
	// m_yout[i] = m_yn[i] * alpha + m_y[i] * beta;
	IBKMK::vector_linear_sum(m_n, alpha, DOUBLE_PTR(m_yn), beta, DOUBLE_PTR(m_y), DOUBLE_PTR(m_yout));
	return &m_yout[0];
}


void IntegratorImplicitEuler::writeStatisticsHeader(const IBK::Path & logfilePath, bool doRestart) {
	if (doRestart) {
		m_statsFileStream = new std::ofstream( (logfilePath + "/integrator_ImplicitEuler_stats.tsv").c_str(), std::ios_base::app);
	}
	else {
		m_statsFileStream = new std::ofstream( (logfilePath + "/integrator_ImplicitEuler_stats.tsv").c_str());
		std::ostream & out = *(m_statsFileStream);
		out << std::setw(25) << std::right << "Time [s]" << "\t";
		out << std::setw(10) << std::right << "Steps" << "\t";
		out << std::setw(10) << std::right << "RhsEvals" << "\t";
		out << std::setw(11) << std::right << "NJacEvals" << "\t";
		out << std::setw(8) << std::right << "NIters" << "\t";
		out << std::setw(11) << std::right << "NDiverged" << "\t";
		out << std::setw(11) << std::right << "NConvFails" << "\t";
		out << std::setw(11) << std::right << "NErrFails" << "\t";
		out << std::setw(14) << std::right << "StepSize [s]";
		out << std::endl;
	}

	m_logFilePath = logfilePath;
}


void IntegratorImplicitEuler::writeStatistics() {
	std::ostream & out = *(m_statsFileStream);
	// Time after last step was completed
	out << std::fixed << std::setprecision(4) << std::setw(25) << std::right << m_t << "\t";
	// Steps
	out << std::fixed << std::setprecision(0) << std::setw(10) << std::right << m_statNumSteps << "\t";
	// RhsEvals
	out << std::fixed << std::setprecision(0) << std::setw(10) << std::right << m_statNumRHSEvals << "\t";
	// NJacEvals
	out << std::fixed << std::setprecision(0) << std::setw(11) << std::right << m_statNumJacEvals << "\t";
	// NIters
	out << std::fixed << std::setprecision(0) << std::setw(8) << std::right << m_statNumIters << "\t";
	// NConvDiverged
	out << std::fixed << std::setprecision(0) << std::setw(11) << std::right << m_statNumConvDiverging << "\t";
	// NConvFails
	out << std::fixed << std::setprecision(0) << std::setw(11) << std::right << m_statNumConvFails << "\t";
	// NErrFails
	out << std::fixed << std::setprecision(0) << std::setw(11) << std::right << m_statNumErrFails << "\t";
	// StepSize
	if (m_dt < 1e-5)
		out << std::setprecision(8) << std::scientific;
	else
		out << std::setprecision(6) << std::fixed;
	out << std::setw(14) << std::right << m_dt;
	out << std::endl;
}


// *** PRIVATE FUNCTIONS ***


void IntegratorImplicitEuler::evaluateModel() {
	// set model state to predicted y values
	ModelInterface::CalculationResult res = m_model->setY( DOUBLE_PTR(m_y) );
	handleErrors(res);

	// and retreive the derivatives estimated by the model for the new time point, store in m_ydot
	res = m_model->ydot( DOUBLE_PTR(m_ydot) ); ++m_statNumRHSEvals;
	handleErrors(res);

	// compute residuals for predicted values
	//	 m_residuals[i] = m_y[i] - m_yn[i] - m_dt*m_ydot[i];
	IBKMK::vector_copy(m_n, DOUBLE_PTR(m_y), DOUBLE_PTR(m_residuals));
	IBKMK::vector_sub(m_n, DOUBLE_PTR(m_yn), DOUBLE_PTR(m_residuals));
	IBKMK::vector_add(m_n, -m_dt, DOUBLE_PTR(m_ydot), DOUBLE_PTR(m_residuals));

	// use either the models error weights function
	if (m_model->hasErrorWeightsFunction()) {
		SOLFRA::ModelInterface::CalculationResult res = m_model->calculateErrorWeights(DOUBLE_PTR(m_y), DOUBLE_PTR(m_weights));
		(void)res; // to make compiler happy
		// ensure validity of the implementation
		IBK_ASSERT(res == SOLFRA::ModelInterface::CalculationSuccess);
	}
	// or equation-dependent absolute tolerances
	else if (m_absTolVec.empty()) {
		for (unsigned int i=0; i<m_n; ++i) {
			m_weights[i] = 1.0/(std::fabs(m_y[i])*m_relTol + m_absTol);
		}
	}
	// or vector of tolerances
	else {
		for (unsigned int i=0; i<m_n; ++i) {
			m_weights[i] = 1.0/(std::fabs(m_y[i])*m_relTol + m_absTolVec[i]);
		}
	}

	// compute current residual norm, scaling the residuals by magnitude of m_y
	m_residualNorm = WRMSNorm(m_residuals);
}


bool IntegratorImplicitEuler::errorTest() {
	const char * const FUNC_ID = "[IntegratorImplicitEuler::errorTest()]";
	// error test is based on a direct evaluation of the first truncated Taylor series term
	// of the implicit euler formulation: err = \frac{\delta_t^2}{2}\ddot{y}^{n+1}
	// where the second derivative is obtained using a Taylor-series expansion on
	// the first derivatives backwards: \ddot{y}^{n+1} = \delta_t \left( \ddot{y}^{n+1} - \dot{y}^{n}\right)
	unsigned int iErrMax = 0;
	double errMax = 0;
	for (unsigned int i=0; i<m_n; ++i) {
		// Note: both variants yield approximately the same error estimate.
		//       However, the second requires the evaluation of the model to compute m_ydot with
		//       the computed solution whereas the first version only requires the states m_y.
		// First version:
		double err = m_errors[i] = std::fabs(0.5*( m_y[i] - m_yPred[i]) );
		// Second version:
		//double err = m_errors[i] = std::fabs(0.5*m_dt*( m_ydot[i] - m_ydotn[i]));
		double relErr = err*m_weights[i];
		if (relErr > errMax) {
			iErrMax = i;
			errMax = relErr;
		}
	}
	m_errNorm = WRMSNorm(m_errors);
	if (m_errNorm >= 1) {
		++m_statNumErrFails;
		IBK::IBK_Message( IBK::FormatString("Error test failed with |err| = %1 (max = %2 in element %3).")
			.arg(m_errNorm).arg(errMax).arg(iErrMax), IBK::MSG_WARNING, FUNC_ID, IBK::VL_DETAILED);

		// reduce time step
		double scale = 1/( std::sqrt(ERROR_NORM_SAFETY*m_errNorm)+0.000001);
		// ensure that scale is reduced at least by SCALE_MIN value
		const double SCALE_MIN = 0.2;
		scale = std::min(scale, SCALE_MIN);
		m_dtNext = m_dt * scale;
		if (m_t + m_dtNext == m_t)
			throw IBK::Exception("Time step too small (rounding errors occur).", FUNC_ID);

		// do not allow increase of time step at end of this step
		m_dtScaleMax = 1;

		// reevaluate model for m_yn at t_n and store now correctly evaluated
		// derivative in m_ydotn for use in next extrapolation
		//IBK::vector_copy(m_n, DOUBLE_PTR(m_yn), DOUBLE_PTR(m_y));
		//evaluateModel();
		//IBK::vector_copy(m_n, DOUBLE_PTR(m_ydot), DOUBLE_PTR(m_ydotn));
	}
	return m_errNorm < 1;
}


void IntegratorImplicitEuler::adjustTimeStep() {
	// we increase the time step based on m_errorNorm, which should be < 1
	IBK_ASSERT(m_errNorm < 1);

	// we enlarge our current error estimate by a certain factor that we assume that
	// the error norm may enlarge due to non-linearities in the next step
	double scale = 1/( std::sqrt(ERROR_NORM_SAFETY*m_errNorm)+0.000001);
	scale = std::min(scale, m_dtScaleMax);
	scale = std::max(1.0, scale);
	const double DT_SCALE_THRESHOLD = 1.5;
	// only allow step size change for significant increase of step sizes
	// as a safeguard against keeping the time step constant forever, only keep it constant for
	// a limited number of steps
	// decrease of time step is not allowed at all
	if (scale < DT_SCALE_THRESHOLD && m_nStepsSinceJacUpdate < 20) {
		scale = 1;
	}

	double dtNew = m_dtNext * scale;
	m_dtNext = std::min(m_dtMax, dtNew);
}


void IntegratorImplicitEuler::determineJacobianUpdate() {
	m_jacUpdateNeeded = false;

	// always update on first call
	if (m_statNumIters == 0) {
		m_jacUpdateNeeded = true;
		return;
	}

	// For all but the ONCE_AT_STEP_START option we always update the Jacobian
	if (m_modifiedNewtonStrategy > MN_ONCE_AT_STEP_START) {
		m_jacUpdateNeeded = true;
		return;
	}

	// if two convergence errors occurred after another
	if (m_newtonConvergenceError) {
		m_jacUpdateNeeded = true;
		return;
	}

	// when time step has changed too much
	double dtRatio = m_dt/m_dtJacUpdate;
	const double MAX_DT_RATIO = 1.3;
	if (dtRatio > MAX_DT_RATIO || 1.0/dtRatio > MAX_DT_RATIO) {
		m_jacUpdateNeeded = true;
		return;
	}

	// when number of steps since Jacobian update exceeds a certain limit and the
	// time step had changed
	if (m_nStepsSinceJacUpdate > 200 && dtRatio != 1) {
		m_jacUpdateNeeded = true;
		return;
	}
}


IntegratorImplicitEuler::StepResult IntegratorImplicitEuler::tryStep() {
	// const char * const FUNC_ID = "[IntegratorImplicitEuler::tryStep]";

	// input values:
	//   m_t holds time point tn
	//   m_y holds predicted solution m_y(pred) at t_{n+1} = m_t + m_dt
	//   m_ydot holds f(m_t+m_dt, m_y(pred))
	//   m_residuals holds corresponding residuals of equations evaluted with m_y and m_ydot
	//   m_residualNorm holds corresponding norm

	//   m_yn holds old solution at t_{n}
	//   m_ydotn holds old time derivative at t_{n}
	//   Jacobian data may be from last or several steps before

	// model is at state m_t, m_y(pred)

	// this flag is true when we have updated the Jacobian matrix at least once in this tryStep() run with the current time step size
	m_jacCurrent = false;

	// *** Jacobian Update ***

	// set m_jacUpdateNeeded to true if we need to recompute the Jacobian
	determineJacobianUpdate();

	// clear Newton convergence error flag
	m_newtonConvergenceError = false;

	// update Jacobian if needed, typically, this is only done once for each Jacobian step when using modified Newton
	if (m_jacUpdateNeeded) {

#ifdef SOLVER_STEP_STATS2
		std::cout << "Jac-update(" << m_dt << ") after " << m_nStepsSinceJacUpdate << " Steps, " << m_nIterations << " Iters" << std::endl;
#endif // SOLVER_STEP_STATS
		// update Jacobian matrix, using first-order finite-difference approximations,
		// also for the estimation of ydot = dy/dt
		// m_ydot contains f(t,y)

		// we are updating the Jacobian with predicted/iterative y values
		SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_LS_SETUP,
			m_lesSolver->setup(DOUBLE_PTR(m_y), DOUBLE_PTR(m_ydot), 0, m_dt);
		);
		++m_statNumJacEvals;

		// reset counters
		m_nStepsSinceJacUpdate = 0;
		m_jacCurrent = true;

		// update Jacobian time step
		m_dtJacUpdate = m_dt;

		// reset convergence rate estimate to 1
		m_convRateDeltaNorm = 1;
	}

	// do Newton iteration and return result
	IntegratorImplicitEuler::StepResult res = newtonIteration();
	if (res == IntegratorImplicitEuler::Success) {
		// we now have m_y set to the new solution, but m_ydot still
		// holds derivative of m_y of last iteration
//#define ALWAYS_REEVALUATE_MODEL_FOR_PREDICTION
#ifdef ALWAYS_REEVALUATE_MODEL_FOR_PREDICTION
		SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_FEVAL,
			evaluateModel(); // calculates m_ydot, m_residuals and m_residualNorm
		);
#else // ALWAYS_REEVALUATE_MODEL_FOR_PREDICTION
		// approximate derivative m_ydot
		//	 m_ydot[i] = (m_y[i] - m_yn[i])/m_dt;
		IBKMK::vector_linear_sum(m_n, 1/m_dt, DOUBLE_PTR(m_y), -1/m_dt, DOUBLE_PTR(m_yn), DOUBLE_PTR(m_ydot));
#endif // ALWAYS_REEVALUATE_MODEL_FOR_PREDICTION
	}
	return res;
}


IntegratorImplicitEuler::StepResult IntegratorImplicitEuler::newtonIteration() {
	const char * const FUNC_ID = "[IntegratorImplicitEuler::newtonIteration]";

	// we monitor the change of vector norms over the whole newton step, and the
	// change between the last update of the Jacobian matrix.
	double residualNormlast = m_residualNorm;
	double deltaNormLast = 1;
	double deltaNorm = 1;
//	double deltaNormPred = 1;

	double convRateResiduals = 1;
	double convRateDeltaNorm = 1;

	// *** Newton-Iteration ***

	// update m_y until vector norm of changes is small enough
	for (;;) {

// enable/disable SOLVER_DEBUG_OUTPUT in header file
#ifdef SOLVER_DEBUG_OUTPUT
		{
			char fname[200];
			FILE * dumpfile;
			sprintf(fname, (m_logFilePath + "/v6_impleuler_y_yn_fy_fyn_res_%d.txt").c_str(), m_statNumRHSEvals);
			dumpfile = fopen(fname, "w");
			for (unsigned int i=0; i<m_n; ++i) {
				fprintf(dumpfile, "%15.2f %10.15g %10.15g %10.15g %10.15g\n", m_y[i], m_yn[i], m_ydot[i], m_ydotn[i], -m_residuals[i]);
			}
			fclose(dumpfile);
		}
#endif // SOLVER_DEBUG_OUTPUT

		// compose right-hand-side of equation system in vector m_deltaY
		//    m_deltaY[i] = -m_residuals[i];
		IBKMK::vector_scale(m_n, -IBKMK_ONE, DOUBLE_PTR(m_residuals), DOUBLE_PTR(m_deltaY));

		// solve equation system, results will be stored in m_deltaY
		SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_LS_SOLVE,
			m_lesSolver->solve(DOUBLE_PTR(m_deltaY));
		);

		// increase iteration counters
		++m_statNumIters; // total Newton iterations
		++m_nIterations;  // local Newton iterations

// enable/disable SOLVER_DEBUG_OUTPUT in header file
#ifdef SOLVER_DEBUG_OUTPUT
		{
			char fname[200];
			FILE * dumpfile;
			sprintf(fname, (m_logFilePath + "/v6_impleuler_b_%d.txt").c_str(), m_statNumRHSEvals);
			dumpfile = fopen(fname, "w");
			for (unsigned int i=0; i<m_n; ++i) {
				fprintf(dumpfile, "%10.15g\n", m_deltaY[i]);
			}
			fclose(dumpfile);
		}
#endif // SOLVER_DEBUG_OUTPUT

		// Since Jacobian matrix is of form: J = I - m_dtJacUpdate*df/dy
		// the solution in m_deltaY will be too large by a factor about m_dt/m_dtJacUpdate
		// and we correct this by multiplying with the inverse ratio m_dtRatioInv
		double dtRatioInv = m_dtJacUpdate/m_dt;
		if (m_dtJacUpdate != m_dt) {
			//   m_deltaY[i] *= dtRatioInv;
			IBKMK::vector_scale_by(m_n, dtRatioInv, DOUBLE_PTR(m_deltaY));
		}

		// compute current norm of changes (delta norm)
		deltaNorm = WRMSNorm(m_deltaY);
//		if (m_nIterations == 1)
//			deltaNormPred = deltaNorm;


		// *** Divergence Check #1 ***
		//
		// out-of-bounds divergence check
		if (deltaNorm > 10000) {
			// we are diverging
			++m_statNumConvDiverging;
			IBK::IBK_Message( IBK::FormatString("Diverging (delta t ratio = %1), "
				"residual/delta norms have become too large!").arg(m_dt/m_dtJacUpdate), IBK::MSG_WARNING, FUNC_ID, IBK::VL_DETAILED);
			if (m_jacCurrent)
				return FailureDiverged;
			else
				return TryAgain;
		}

		// compute new solution, y_{m+1} = y_m + deltaY
//#define IMPLICIT_EULER_USE_CLIPPING
#ifdef IMPLICIT_EULER_USE_CLIPPING
		for (unsigned int i=0; i<m_n; ++i) {
			double yNew = m_y[i] + m_deltaY[i];
			if (yNew > m_yMax[i]) {
				yNew = m_yMax[i];
			}
			else if (yNew < m_yMin[i]) {
				yNew = m_yMin[i];
			}
			m_y[i] = yNew;
		}
#else // IMPLICIT_EULER_USE_CLIPPING
		//  m_y[i] += m_deltaY[i];
		IBKMK::vector_add(m_n, IBKMK_ONE, DOUBLE_PTR(m_deltaY), DOUBLE_PTR(m_y));
#endif // IMPLICIT_EULER_USE_CLIPPING

		// *** Convergence Check #1 ***
		//
		convRateDeltaNorm = deltaNorm/deltaNormLast;
		// update Jacobian-based convergence rate estimate
		const double CONVERGENCE_RATE_REDUCTION_FACTOR = 0.3;
		if (m_nIterations > 1)
			m_convRateDeltaNorm = std::max(CONVERGENCE_RATE_REDUCTION_FACTOR*m_convRateDeltaNorm, deltaNorm/deltaNormLast);
		// deltaNorm must be less than m_nonLinConvCoeff and the factor 0.5 (from the error test)
		double deltaNormScaled = std::min(1.0, m_convRateDeltaNorm)*deltaNorm/m_nonLinConvCoeff*0.5;

		// TODO : add protection against convergence stall
		//        accept solution as converged if sufficient progress towards the solution has been made
		if (deltaNormScaled <= 1.0) {
			// write iteration statistics
			writeIterationStats(0, deltaNorm, convRateDeltaNorm);
			break; // converged
		}

		// *** Iteration limit check ***
		if (m_nIterations >= m_maxNonLinIters) {
			// doesn't work, only creates more error failes
			//if (m_residualNorm/residualNormPred < 0.1 || deltaNorm/deltaNormPred < 0.1) {
			//	break; // converged?
			//}
			++m_statNumConvFails;
			// reset iteration counter
			m_nIterations = 0;
			if (m_jacCurrent) {
				IBK::IBK_Message( "Too many Newton iterations, reducing time step!", IBK::MSG_WARNING, FUNC_ID, IBK::VL_DETAILED);
				return FailureIterationLimitExceeded;
			}

			// signal to update Jacobian
			m_newtonConvergenceError = true;
			IBK::IBK_Message( IBK::FormatString("Too many Newton iterations (delta t ratio = %1), "
				"but we retry with Jacobian computed for predicted solution!").arg(m_dt/m_dtJacUpdate), IBK::MSG_WARNING, FUNC_ID, IBK::VL_DETAILED);
			return TryAgain;
		}

		// Newton-Step done, evaluate model for new solution
		SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_FEVAL,
			evaluateModel(); // calculates m_ydot, m_residuals and m_residualNorm
		);

		// compute convergence rates
		convRateResiduals = m_residualNorm/residualNormlast;

		// store norms for next iteration
		residualNormlast = m_residualNorm;
		deltaNormLast = deltaNorm;

		// write iteration statistics
		writeIterationStats(convRateResiduals, deltaNorm, convRateDeltaNorm);

		// we have had at least DIVERGENCE_CHECK_ITER_LIMIT iterations since start of Newton Step
		// so we can compute convergence rates
		const unsigned int DIVERGENCE_CHECK_ITER_LIMIT = 3;
		if (m_nIterations > DIVERGENCE_CHECK_ITER_LIMIT) {
			// if either the residual or the delta norms are diverging, try again
			if (convRateResiduals > 1 && convRateDeltaNorm > 1) {
				// we are diverging

				IBK::IBK_Message( IBK::FormatString("Diverging (delta t ratio = %1)!").arg(m_dt/m_dtJacUpdate), IBK::MSG_WARNING, FUNC_ID, IBK::VL_DETAILED);
				++m_statNumConvDiverging;
				if (m_jacCurrent) {
					return FailureDiverged;
				}
				else {
					return TryAgain;
				}
			}
		} // if (m_nIterations > 1)

		// for strict Newton variant, update Jacobian matrix, but DO NOT reset convergence rate
		if ((m_modifiedNewtonStrategy == MN_EVERY_ITERATION) ||
			(m_modifiedNewtonStrategy == MN_EVERY_ITERATION_UNTIL_THRESHOLD && m_residualNorm > 0.01))
		{
			m_lesSolver->setup(DOUBLE_PTR(m_y), DOUBLE_PTR(m_ydot), 0, m_dt);
			++m_statNumJacEvals;

			// reset counters
			m_nStepsSinceJacUpdate = 0;
			m_jacCurrent = true;

			// update Jacobian time step
			m_dtJacUpdate = m_dt;
		}

	} // while iteration
	++m_nStepsSinceJacUpdate;

	return Success;
}


void IntegratorImplicitEuler::decreaseTimeStepNonLinearFailure() {
	// half the time step
	m_dtNext = m_dt/2;
	if (m_t + m_dt == m_t)
		throw IBK::Exception("Time step too small (rounding errors occur).", "[IntegratorImplicitEuler::decreaseTimeStepNonLinearFailure]");
}


void IntegratorImplicitEuler::handleErrors(ModelInterface::CalculationResult res) const {
	if (res == ModelInterface::CalculationAbort)
		throw IBK::Exception("Aborting simulation (signaled from within model).", "[IntegratorImplicitEuler::handleErrors]");
}


double IntegratorImplicitEuler::WRMSNorm(std::vector<double> & vec) const {
	double r = 0;
	// three variants for computing Euclidian Norm:
	// 1. do not divide by m_n at all
	// 2. divide r_normalized by m_n
	// 3. divide r by m_n
	// assume m_n = 100
	// 1 - gives error of order 1e-5
	// 2 - gives error of order 1e-7
	// 3 - gives error of order 1e-6
	for (unsigned int i=0; i<m_n; ++i) {
		double r_normalized = vec[i]*m_weights[i];
//		double r_normalized = vec[i]*m_weights[i]/m_n; // Option #2
		r += r_normalized*r_normalized;
	}
	r = std::sqrt(r/m_n); // Option #3
//	r = std::sqrt(r); // Option #1
	return r;
}


void IntegratorImplicitEuler::writeIterationStats(double convRateResiduals, double deltaNorm, double convRateDeltaNorm) const {
	const char * const FUNC_ID = "[IntegratorImplicitEuler::writeIterationStats]";
	// on first iteration print headers
#ifdef SOLVER_STEP_STATS
	std::ofstream dump( (m_logFilePath + "/iter_stats.txt").c_str(), std::ios_base::app);
#endif // SOLVER_STEP_STATS
	if (m_statNumIters == 0 && m_statNumSteps == 0) {
#ifdef SOLVER_STEP_STATS
		dump.close();
		dump.open( (m_logFilePath + "/iter_stats.txt").c_str() );
#endif // SOLVER_STEP_STATS
		std::stringstream strm;
		strm << std::setw(6) << std::left << "Steps "
					<< std::setw(7) << std::left << "TIters "
					<< std::setw(11) << std::left << "t_{n+1} "
					<< std::setw(11) << std::left << "dt "
					<< std::setw(7) << std::left << "NIters "
					<< std::setw(7) << std::left << "NItJac "
					<< std::setw(13) << std::left << "NStepsJac "
					<< std::setw(13) << std::left << "JacCurrent "
					<< std::setw(12) << std::left << "ResNorm "
					<< std::setw(12) << std::left << "CRateRes "
					<< std::setw(12) << std::left << "DeltaNorm "
					<< std::setw(14) << std::left << "CRateDelta "
					<< std::endl;
		IBK::IBK_Message( strm.str(), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DETAILED);
#ifdef SOLVER_STEP_STATS
		dump << strm.str();
#endif // SOLVER_STEP_STATS
	}

	std::stringstream strm_dump;
	strm_dump << std::setw(6) << std::left << m_statNumSteps
				<< std::setw(7) << std::left << m_statNumIters
				<< std::setw(11) << std::left << IBK::Time::format_time_difference(m_t + m_dt)
				<< std::setw(11) << std::left << m_dt
				<< std::setw(7) << std::left << m_nIterations
				<< std::setw(7) << std::left << m_nItersSinceJacUpdate
				<< std::setw(13) << std::left << m_nStepsSinceJacUpdate
				<< std::setw(13) << std::left << std::boolalpha << m_jacUpdateNeeded
				<< std::setw(12) << std::setprecision(3) << std::left << m_residualNorm
				<< std::setw(12) << std::setprecision(3) << std::left << convRateResiduals
				<< std::setw(12) << std::setprecision(3) << std::left << deltaNorm
				<< std::setw(14) << std::setprecision(3) << std::left << convRateDeltaNorm
				<< std::endl;
	IBK::IBK_Message( strm_dump.str(), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DETAILED);

#ifdef SOLVER_STEP_STATS
	dump << strm_dump.str();
#endif // SOLVER_STEP_STATS
}


void IntegratorImplicitEuler::writeMetrics(double simtime, std::ostream * metricsFile) {
	const char * const FUNC_ID = "[IntegratorImplicitEuler::writeMetrics]";
	std::string ustr = IBK::Time::suitableTimeUnit(simtime);
	IBK::IBK_Message( IBK::FormatString("Integrator: Steps                          =                          %1\n")
		.arg(m_statNumSteps,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("Integrator: Newton iterations              =                          %1\n")
		.arg(m_statNumIters,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("Integrator: Newton convergence failures    =                          %1\n")
		.arg(m_statNumConvFails + m_statNumConvDiverging,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("Integrator: Error test failures            =                          %1\n")
		.arg(m_statNumErrFails,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	double tfeval = TimerSum(SUNDIALS_TIMER_FEVAL);
	IBK::IBK_Message( IBK::FormatString("Integrator: Function evaluation (Newton)   = %1 (%2 %%)  %3\n")
		.arg(IBK::Time::format_time_difference(tfeval, ustr, true),13)
		.arg(tfeval/simtime*100, 5, 'f', 2)
		.arg(m_statNumRHSEvals,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	double tsetup = TimerSum(SUNDIALS_TIMER_LS_SETUP);
	IBK::IBK_Message( IBK::FormatString("Integrator: LES setup                      = %1 (%2 %%)  %3\n")
		.arg(IBK::Time::format_time_difference(tsetup, ustr, true),13)
		.arg(tsetup/simtime*100, 5, 'f', 2)
		.arg(m_statNumJacEvals,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	double tsolve = TimerSum(SUNDIALS_TIMER_LS_SOLVE);
	IBK::IBK_Message( IBK::FormatString("Integrator: LES solve                      = %1 (%2 %%)  %3\n")
		.arg(IBK::Time::format_time_difference(tsolve, ustr, true),13)
		.arg(tsolve/simtime*100, 5, 'f', 2)
		.arg(m_statNumIters,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	if (metricsFile != nullptr) {
		*metricsFile << "IntegratorSteps=" << m_statNumSteps << std::endl;
		*metricsFile << "IntegratorNewtonConvFails=" << m_statNumConvFails + m_statNumConvDiverging << std::endl;
		*metricsFile << "IntegratorErrorTestFails=" << m_statNumErrFails << std::endl;
		*metricsFile << "IntegratorFunctionEvals=" << m_statNumRHSEvals << std::endl;
		*metricsFile << "IntegratorTimeFunctionEvals=" << tfeval << std::endl;
		*metricsFile << "IntegratorLESSetup=" << m_statNumJacEvals << std::endl;
		*metricsFile << "IntegratorTimeLESSetup=" << tsetup << std::endl;
		*metricsFile << "IntegratorLESSolve=" << m_statNumIters << std::endl;
		*metricsFile << "IntegratorTimeLESSolve=" << tsolve << std::endl;
	}
}

} // namespace SOLFRA
