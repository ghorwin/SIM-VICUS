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

#include "SOLFRA_IntegratorExplicitEuler.h"

#include <iostream>
#include <iomanip>
#include <cmath>
#include <fstream>
#include <algorithm>

#include <IBK_Exception.h>
#include <IBK_messages.h>
#include <IBK_assert.h>
#include <IBK_Time.h>

#include <sundials/sundials_timer.h>

#include "SOLFRA_ModelInterface.h"

namespace SOLFRA {

IntegratorExplicitEuler::IntegratorExplicitEuler() :
	m_dt(0),
	m_statsFileStream(nullptr),
	m_statNumSteps(0),
	m_statNumRHSEvals(0)
{
}


void IntegratorExplicitEuler::init(	ModelInterface * model,
									double t0,
									const double * y0,
									LESInterface * /*lesSolver*/,
									PrecondInterface * /*precond*/,
									JacobianInterface * /*jacobian*/)
{
	FUNCID(IntegratorExplicitEuler::init);

	IBK_Message( "Initializing Explicit Euler integrator\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	m_model = model;
	IBK_ASSERT(m_model != nullptr);

	m_t = t0;
	// if user didn't specify time step, take the value from the model
	if (m_dt == 0)
		m_dt = model->dt0();
	m_n=model->n();
	m_y.resize(m_n);
	m_ydot.resize(m_n);
	m_yn.resize(m_n);
	m_yout.resize(m_n);
	m_res.resize(m_n);
	// transfer initial conditions
	std::copy(y0, y0+m_n, &m_y[0]);

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


IntegratorInterface::StepResultType IntegratorExplicitEuler::step() {
	double tnew = m_t + m_dt;
	ModelInterface::CalculationResult res;
	SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_FEVAL,
		res = m_model->setTime(tnew);
	);
	if (res != ModelInterface::CalculationSuccess) {
		if (res == ModelInterface::CalculationRecoverableError)
			return IntegratorInterface::StepAttemptRestart;
		else
			return IntegratorInterface::StepCriticalError; // stop Integrator
	}

	// retrieve divergences
	SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_FEVAL,
		res = m_model->ydot(&m_ydot[0]);
	);
	if (res != ModelInterface::CalculationSuccess) {
		if (res == ModelInterface::CalculationRecoverableError)
			return IntegratorInterface::StepAttemptRestart;
		else
			return IntegratorInterface::StepCriticalError; // stop Integrator
	}

	// store old solution
	m_yn.swap(m_y); // y -> yn
	// compute new solution
	for (unsigned int i=0; i<m_n; ++i) {
		m_y[i] = m_yn[i] + m_dt*m_ydot[i];
	}
	// set state of model to new values
	SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_FEVAL,
		res = m_model->setY(&m_y[0]); ++m_statNumRHSEvals;
	);
	if (res != ModelInterface::CalculationSuccess) {
		if (res == ModelInterface::CalculationRecoverableError)
			return IntegratorInterface::StepAttemptRestart;
		else
			return IntegratorInterface::StepCriticalError; // stop Integrator
	}

	// advance solution in time
	m_t = tnew;
	++m_statNumSteps;
	return IntegratorInterface::StepSuccess;
}


const double * IntegratorExplicitEuler::yOut(double t_out) const {
	if (t_out == m_t)
		return &m_y[0];
	// interpolate between last step and current step
	double t = std::min(t_out, m_t); // we don't allow extrapolations
	double alpha = (m_t-t)/m_dt;
	double beta = 1-alpha;
	for (unsigned int i=0; i<m_n; ++i) {
		m_yout[i] = m_yn[i] * alpha + m_y[i] * beta;
	}
	return &m_yout[0];
}


void IntegratorExplicitEuler::writeStatisticsHeader(const IBK::Path & logfilePath, bool doRestart) {
	if (doRestart) {
		m_statsFileStream = new std::ofstream( (logfilePath + "/integrator_ExplicitEuler_stats.tsv").c_str(), std::ios_base::app);
	}
	else {
		m_statsFileStream = new std::ofstream( (logfilePath + "/integrator_ExplicitEuler_stats.tsv").c_str());
		std::ostream & out = *(m_statsFileStream);
		out << std::setw(25) << std::right << "Time [s]\t";
		out << std::setw(7) << std::right << "NSteps\t";
		out << std::setw(7) << std::right << "NRhsEvals\t";
		out << std::setw(14) << std::right << "StepSize [s]";
		out << std::endl;
	}
}


void IntegratorExplicitEuler::writeStatistics() {
	std::ostream & out = *(m_statsFileStream);
	// Time after last step was completed
	out << std::fixed << std::setprecision(4) << std::setw(25) << std::right << m_t << "\t";
	// NSteps
	out << std::fixed << std::setprecision(0) << std::setw(7) << std::right << m_statNumSteps << "\t";
	// NRhsEvals
	out << std::fixed << std::setprecision(0) << std::setw(10) << std::right << m_statNumRHSEvals << "\t";
	// StepSize
	if (m_dt < 1e-5)
		out << std::setprecision(5) << std::scientific;
	else
		out << std::setprecision(4) << std::fixed;
	out << std::setw(14) << std::right << m_dt;
	out << std::endl;
	// Note: Do not write and endl into the stream!
}


void IntegratorExplicitEuler::writeMetrics(double simtime, std::ostream * metricsFile) {
	FUNCID(IntegratorExplicitEuler::writeMetrics);
	std::string ustr = IBK::Time::suitableTimeUnit(simtime);
	IBK::IBK_Message( IBK::FormatString("Integrator: Steps                          =                          %1\n")
		.arg(m_statNumSteps,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	double tfeval = TimerSum(SUNDIALS_TIMER_FEVAL);
	IBK::IBK_Message( IBK::FormatString("Integrator: Function evaluation            = %1 (%2 %%)  %3\n")
		.arg(IBK::Time::format_time_difference(tfeval, ustr, true),13)
		.arg(tfeval/simtime*100, 5, 'f', 2)
		.arg(m_statNumRHSEvals,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	if (metricsFile != nullptr) {
		*metricsFile << "IntegratorSteps=" << m_statNumSteps << std::endl;
		*metricsFile << "IntegratorFunctionEvals=" << m_statNumRHSEvals << std::endl;
		*metricsFile << "IntegratorTimeFunctionEvals=" << tfeval << std::endl;
	}
}


} // namespace SOLFRA
