#include "SOLFRA_IntegratorRungeKutta45.h"

#include <iostream>
#include <iomanip>
#include <cmath>
#include <fstream>
#include <algorithm>

#include <IBK_Exception.h>
#include <IBK_messages.h>
#include <IBK_Time.h>
#include <IBK_assert.h>
#include <IBK_FileUtils.h>

#include <sundials/sundials_timer.h>

#include "SOLFRA_ModelInterface.h"

namespace SOLFRA {

IntegratorRungeKutta45::IntegratorRungeKutta45(double dt0) :
	m_dtMax(3600),
	m_dt(dt0),
	m_statsFileStream(nullptr),
	m_statNumSteps(0),
	m_statNumRHSEvals(0),
	m_statNumFails(0)
{
}

void IntegratorRungeKutta45::init(ModelInterface * model, double t0, const double * y0,
	LESInterface * /*lesSolver*/, PrecondInterface * /*precond*/, JacobianInterface * /*jacobian*/)
{
	const char * const FUNC_ID = "[IntegratorRungeKutta45::init]";

	m_model = model;
	IBK_ASSERT(m_model != nullptr);


	m_t = t0;

	// if user didn't specify time step, take the value from the model
	if (m_dt == 0)
		m_dt = m_model->dt0();
	IBK::IBK_Message(IBK::FormatString("t = %1, dt = %2\n").arg(m_t).arg(m_dt), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DEVELOPER);

	m_dtProposed = m_dt;
	m_lastErr = 1e-20;
	m_n=m_model->n();

	m_yout.resize(m_n);
	m_y.resize(m_n);
	m_yn.resize(m_n);
	m_ynext.resize(m_n);
	m_ydot.resize(m_n);
	m_ydotnext.resize(m_n);
	m_alpha2.resize(m_n);
	m_alpha3.resize(m_n);
	m_alpha4.resize(m_n);
	m_alpha5.resize(m_n);
	m_alpha6.resize(m_n);
	m_err.resize(m_n);

	// transfer initial conditions
	std::copy(y0, y0+m_n, &m_y[0]);
	std::copy(y0, y0+m_n, &m_yn[0]);


	// initialize state of model to initial conditions
	ModelInterface::CalculationResult res = m_model->setTime(m_t);
	if (res != ModelInterface::CalculationSuccess)
		throw IBK::Exception("Error initializing the model: failed setting the start time.", FUNC_ID);

	res = m_model->setY(&m_y[0]); ++m_statNumRHSEvals;
	if (res != ModelInterface::CalculationSuccess)
		throw IBK::Exception("Error initializing the model: failed setting the initial states.", FUNC_ID);

	res = m_model->ydot(&m_ydot[0]);
	if (res != ModelInterface::CalculationSuccess)
		throw IBK::Exception("Error initializing the model: failed retrieving time derivatives of initial solution.", FUNC_ID);

#ifdef SUNDIALS_USE_INSTRUMENTATION
	for (int i=0; i<SUNDIALS_TIMER_COUNT; ++i)
		TimerSumReset(i);
#endif // SUNDIALS_USE_INSTRUMENTATION

}


IntegratorInterface::StepResultType IntegratorRungeKutta45::step() {
	const char * const FUNC_ID = "[IntegratorRungeKutta45::step]";
	IBK::IBK_Message(IBK::FormatString("step() t = %1, dt = %2\n").arg(m_t).arg(m_dt), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DEVELOPER);
	// we expect m_ydot to hold derivatives at current time point m_t
	// m_y contains values at time point m_t
	// m_dt holds time step last taken
	// m_dtProposed holds new time step proposal

	// trial loop, we leave if the step succeeded
	bool repeat = true;
	double err = 0; // initialization only necessary to avoid compiler warning
	while (repeat) {
		// evaluate function at m_t + m_dtProposed
		// populates m_ynext and m_ydotnext as well as m_err
		trialStep(m_dtProposed);

		// calculate error norm
		unsigned int i_max;
		err = errorNorm(i_max);
//		if (err > 1)
//			cout << "i = " << i_max << " m_dtProposed = " << m_dtProposed << endl;

		// loop conditions uses controller which adjusts adjust step size and returns
		// true if step needs to be repeated
		repeat = adjustStepSize(err);
		// if repeat = false, m_dt is set to m_dtProposed and m_dtProposed is adjusted to a new value
		// otherwise only m_dtProposed is changed
	}
	++m_statNumSteps;

	// m_dt contains last step size, m_dtProposed contains new step size for next step
	m_lastErr = err;

	// advance step and do bookkeeping
	m_t += m_dt;
	m_yn.swap(m_y);	// m_y -> m_yn
	m_y.swap(m_ynext);	// m_ynext -> m_y
	m_ydot.swap(m_ydotnext); // m_ydotnext -> m_ydot
	return IntegratorInterface::StepSuccess;
}


const double * IntegratorRungeKutta45::yOut(double t_out) const {
	if (t_out == m_t)
		return &m_y[0];
	// TODO : research interpolation method for RK45
	// interpolate between last step and current step
	double t = std::min(t_out, m_t); // we don't allow extrapolations
	double alpha = (m_t-t)/m_dt;
	double beta = 1-alpha;
	for (unsigned int i=0; i<m_n; ++i) {
		m_yout[i] = m_yn[i] * alpha + m_y[i] * beta;
	}
	return &m_yout[0];
}


void IntegratorRungeKutta45::writeStatisticsHeader(const IBK::Path & logfilePath, bool doRestart) {
	if (doRestart) {
		m_statsFileStream = IBK::create_ofstream(logfilePath / "integrator_RK45_stats.tsv", std::ios_base::app);
	}
	else {
		m_statsFileStream = IBK::create_ofstream(logfilePath / "integrator_RK45_stats.tsv");
		std::ostream & out = *(m_statsFileStream);
		out << std::setw(25) << std::right << "Time [s]\t";
		out << std::setw(7) << std::right << "NSteps\t";
		out << std::setw(7) << std::right << "NRhsEvals\t";
		out << std::setw(7) << std::right << "NFails\t";
		out << std::setw(14) << std::right << "StepSize [s]";
		out << std::endl;
	}
}


void IntegratorRungeKutta45::writeStatistics() {
	std::ostream & out = *(m_statsFileStream);
	// Time after last step was completed
	out << std::fixed << std::setprecision(4) << std::setw(25) << std::right << m_t << "\t";
	// NSteps
	out << std::fixed << std::setprecision(0) << std::setw(7) << std::right << m_statNumSteps << "\t";
	// NRhsEvals
	out << std::fixed << std::setprecision(0) << std::setw(10) << std::right << m_statNumRHSEvals << "\t";
	// NFails
	out << std::fixed << std::setprecision(0) << std::setw(10) << std::right << m_statNumFails << "\t";
	// StepSize
	if (m_dt < 1e-5)
		out << std::setprecision(5) << std::scientific;
	else
		out << std::setprecision(4) << std::fixed;
	out << std::setw(14) << std::right << m_dt;
	out << std::endl;
}


void IntegratorRungeKutta45::writeMetrics(double simtime, std::ostream * metricsFile) {
	const char * const FUNC_ID = "[IntegratorRungeKutta45::writeMetrics]";
	std::string ustr = IBK::Time::suitableTimeUnit(simtime);
	IBK::IBK_Message( IBK::FormatString("Integrator: Steps                          =                          %1\n")
		.arg(m_statNumSteps,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("Integrator: Error test failures            =                          %1\n")
		.arg(m_statNumFails,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	double tfeval = TimerSum(SUNDIALS_TIMER_FEVAL);
	IBK::IBK_Message( IBK::FormatString("Integrator: Function evaluation            = %1 (%2 %%)  %3\n")
		.arg(IBK::Time::format_time_difference(tfeval, ustr, true),13)
		.arg(tfeval/simtime*100, 5, 'f', 2)
		.arg(m_statNumRHSEvals,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	if (metricsFile != nullptr) {
		*metricsFile << "IntegratorSteps=" << m_statNumSteps << std::endl;
		*metricsFile << "IntegratorErrorTestFails=" << m_statNumFails << std::endl;
		*metricsFile << "IntegratorFunctionEvals=" << m_statNumRHSEvals << std::endl;
		*metricsFile << "IntegratorTimeFunctionEvals=" << tfeval << std::endl;
	}
}


// *** Private Member Functions ***

bool IntegratorRungeKutta45::trialStep(double dt) {
	// Runge-Kutta Dormand-Prince constants
	static const double c2 = 1.0/5;
	static const double c3 = 3.0/10;
	static const double c4 = 4.0/5;
	static const double c5 = 8.0/9;
	static const double c6 = 1;
	static const double c7 = 1;

	static const double a21 = 1.0/5;

	static const double a31 = 3.0/40;
	static const double a32 = 9.0/40;

	static const double a41 = 44.0/45;
	static const double a42 = -56.0/15;
	static const double a43 = 32.0/9;

	static const double a51 = 19372.0/6561.0;
	static const double a52 = -25360.0/2187.0;
	static const double a53 = 64448.0/6561;
	static const double a54 = -212.0/729;

	static const double a61 = 9017.0/3168;
	static const double a62 = -355.0/33;
	static const double a63 = 46732.0/5247;
	static const double a64 = 49.0/176;
	static const double a65 = -5103.0/18656;

	static const double a71 = 35.0/384;
	// static const double a72 = 0;
	static const double a73 = 500.0/1113;
	static const double a74 = 125.0/192;
	static const double a75 = -2187.0/6784;
	static const double a76 = 11.0/84;

	static const double e1 = 71.0/57600.0;
	// static const double e2 = 0;
	static const double e3 = -71.0/16695;
	static const double e4 = 71.0/1920;
	static const double e5 = -17253.0/339200.0;
	static const double e6 = 22.0/525.0;
	static const double e7 = -1.0/40.0;

	// go through the steps
	// for the first step we expect m_ydot to contain the derivatives
	// for m_y, both at time m_t

	// first evaluation, no need to call model yet, we use first-as-last approach
	for (unsigned int i=0; i<m_n; ++i)
		m_ynext[i] = m_y[i] + dt*a21*m_ydot[i];

	// second evaluation
	SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_FEVAL,
		if (evaluateModel(m_t + c2*dt, &m_ynext[0], &m_alpha2[0]) != 0) return false; ++m_statNumRHSEvals;
	);

	for (unsigned int i=0; i<m_n; ++i)
		m_ynext[i] = m_y[i] + dt*(a31*m_ydot[i] + a32*m_alpha2[i]);

	// third evaluation
	SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_FEVAL,
		if (evaluateModel(m_t + c3*dt, &m_ynext[0], &m_alpha3[0]) != 0) return false; ++m_statNumRHSEvals;
	);
	for (unsigned int i=0; i<m_n; ++i)
		m_ynext[i] = m_y[i] + dt*(a41*m_ydot[i] + a42*m_alpha2[i] + a43*m_alpha3[i]);

	// fourth evaluation
	SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_FEVAL,
		if (evaluateModel(m_t + c4*dt, &m_ynext[0], &m_alpha4[0]) != 0) return false; ++m_statNumRHSEvals;
	);
	for (unsigned int i=0; i<m_n; ++i)
		m_ynext[i] = m_y[i] + dt*(a51*m_ydot[i] + a52*m_alpha2[i] + a53*m_alpha3[i] + a54*m_alpha4[i]);

	// fifth evaluation
	SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_FEVAL,
		if (evaluateModel(m_t + c5*dt, &m_ynext[0], &m_alpha5[0]) != 0) return false; ++m_statNumRHSEvals;
	);
	for (unsigned int i=0; i<m_n; ++i)
		m_ynext[i] = m_y[i] + dt*(a61*m_ydot[i] + a62*m_alpha2[i] + a63*m_alpha3[i] + a64*m_alpha4[i] + a65*m_alpha5[i]);

	// sixth evaluation
	SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_FEVAL,
		if (evaluateModel(m_t + c6*dt, &m_ynext[0], &m_alpha6[0]) != 0) return false; ++m_statNumRHSEvals;
	);
	// now create the final y as weighted sum of all intermediate values
	for (unsigned int i=0; i<m_n; ++i)
		m_ynext[i] = m_y[i] + dt*(a71*m_ydot[i] + a73*m_alpha3[i] + a74*m_alpha4[i] + a75*m_alpha5[i] + a76*m_alpha6[i]);

	// final evaluation, will also be the first step in the next round
	// (FSAL - First Same As Last)
	SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_FEVAL,
		if (evaluateModel(m_t + c7*dt, &m_ynext[0], &m_ydotnext[0]) != 0) return false; ++m_statNumRHSEvals;
	);

	// calculate error estimate as difference between fourth and fifth order
	for (unsigned int i=0; i<m_n; ++i)
		m_err[i] = dt*(e1*m_ydot[i] + e3*m_alpha3[i] + e4*m_alpha4[i] + e5*m_alpha5[i] + e6*m_alpha6[i] + e7*m_ydotnext[i]);

	return true;
}


int IntegratorRungeKutta45::evaluateModel(double t, const double * y, double * ydot) const {
	// initialize state of model to initial conditions
	ModelInterface::CalculationResult res = m_model->setTime(t);
	if (res != ModelInterface::CalculationSuccess) {
		return res;
	}
	res = m_model->setY(y);
	if (res != ModelInterface::CalculationSuccess) {
		return res;
	}
	res = m_model->ydot(ydot);
	if (res != ModelInterface::CalculationSuccess) {
		return res;
	}
	return 0;
}


double IntegratorRungeKutta45::errorNorm(unsigned int & i_max) const {
	// use some calculation method for the error norm

	double err = 0;
	double max = 0;
	double scale;

	if (m_absTolVec.empty()) {
		for (unsigned int i=0; i<m_n; ++i) {
			scale = m_absTol + m_relTol*std::max(fabs(m_y[i]),fabs(m_ynext[i]));
			double local_err = m_err[i]*m_err[i]/(scale*scale);
			if (local_err > max) {
				i_max = i;
				max = local_err;
			}
			err += local_err;
		}
	} else {
		for (unsigned int i=0; i<m_n; ++i) {
			scale = m_absTolVec[i] + m_relTol*std::max(fabs(m_y[i]),fabs(m_ynext[i]));
			double local_err = m_err[i]*m_err[i]/(scale*scale);
			if (local_err > max) {
				i_max = i;
				max = local_err;
			}
			err += local_err;
		}
	}

	err = std::max(1e-20, std::sqrt(err/m_n));

	return err;
}

bool IntegratorRungeKutta45::adjustStepSize(double err) {
	const char * const FUNC_ID = "[IntegratorRungeKutta45::adjustStepSize]";
	// basic implementation, weighting factor PI turns on proportional-integral controller
	static const double PI_WEIGHT = 0.04; // set to 0.04 .. 0.08
	static const double PI_WEIGHT_COMPLEMENT = 0.2 - PI_WEIGHT*0.75;
	static const double SAFETY = 0.9;
	static const double MINSCALE = 0.2;
	static const double MAXSCALE = 20.0;

	double scale; // will contain the scaling factor
	// success is indicated if weighted error is <= unity
	if (err <= 1.0) {
		// success, increase step size
		if (err == 0.0)
			scale = MAXSCALE;
#define USE_PI_CONTROLLER
#ifdef USE_PI_CONTROLLER
		else {
			scale = std::pow(err, -PI_WEIGHT_COMPLEMENT);
			// multiply integral part to controller
			if (PI_WEIGHT != 0)
				scale *= std::pow(m_lastErr, PI_WEIGHT);
		}
#else
		else {
			scale = std::pow(1.0/(err+1e-10), 0.2);
		}
#endif
		// store successful step size
		m_dt = m_dtProposed;
		// compute new estimate for step size
		scale *= SAFETY; // multiply with safety factor to reduce risk or failures
		m_dtProposed *= std::min(MAXSCALE,scale);

		m_dtProposed = std::min(m_dtProposed, m_dtMax);
		IBK::IBK_Message(IBK::FormatString("err = %1, dtProposed = %2\n").arg(err).arg(m_dtProposed), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DEVELOPER);
		return false; // no retry necessary
	}
	else {
		// failure, decrease step size
		scale = SAFETY*std::pow(err, -PI_WEIGHT_COMPLEMENT);
		m_dtProposed *= std::max(MINSCALE,scale);
		IBK::IBK_Message(IBK::FormatString("err = %1, dtProposed = %2 - repeat\n").arg(err).arg(m_dtProposed), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DEVELOPER);
		++m_statNumFails;
		return true; // retry needed
	}
}

} // namespace SOLFRA
