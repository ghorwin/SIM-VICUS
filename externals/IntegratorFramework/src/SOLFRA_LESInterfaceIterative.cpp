#include "SOLFRA_LESInterfaceIterative.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <fstream>

#include <IBK_assert.h>
#include <IBK_Time.h>
#include <IBK_messages.h>

#include <cvode/cvode_spils.h>
#include <sundials/sundials_timer.h>

#include "SOLFRA_IntegratorSundialsCVODE.h"
#include "SOLFRA_JacobianInterface.h"
#include "SOLFRA_PrecondInterface.h"

namespace SOLFRA {

void LESInterfaceIterative::writeStatisticsHeader(const std::string & logfilePath, bool doRestart ) {

	if (doRestart) {
		m_statsFileStream = new std::ofstream( (logfilePath + "/LES_iterative_stats.tsv").c_str(), std::ios_base::app);
	}
	else {
		m_statsFileStream = new std::ofstream( (logfilePath + "/LES_iterative_stats.tsv").c_str());
		std::ostream & out = *m_statsFileStream;
		out << std::setw(25) << std::right << "Time\t";
		out << std::setw(10) << std::right << "NLIters\t";
		out << std::setw(10) << std::right << "NLCFails\t";
		out << std::setw(10) << std::right << "NPEvals\t";
		out << std::setw(10) << std::right << "NPSolve\t";
		out << std::setw(13) << std::right << "NJvEvals\t";
		out << std::setw(13) << std::right << "NRhsEvals";
		out << std::endl;
	}

}


void LESInterfaceIterative::writeStatistics(double t) {
	updateSUNDIALSStatistics();

	// append to output file
	std::ostream & out = *m_statsFileStream;

	// Time after last step was completed
	out << std::fixed << std::setprecision(10) << std::setw(25) << std::right << t << "\t";
	// NLIters
	out << std::fixed << std::setprecision(0) << std::setw(10) << std::right << m_statNumLinIters << "\t";
	// NLCFails
	out << std::fixed << std::setprecision(0) << std::setw(10) << std::right << m_statNumLinConvFails << "\t";
	// NPrecEvals
	out << std::fixed << std::setprecision(0) << std::setw(10) << std::right << m_statNumPrecEvals << "\t";
	// NPrecSolve
	out << std::fixed << std::setprecision(0) << std::setw(10) << std::right << m_statNumPrecSolves << "\t";
	// NJtimesEvals
	out << std::fixed << std::setprecision(0) << std::setw(13) << std::right << m_statNumJacTimesEvals << "\t";
	// NRhsEvals
	out << std::fixed << std::setprecision(0) << std::setw(13) << std::right << m_statNumRhsEvals;
	out << std::endl;
}


void LESInterfaceIterative::updateSUNDIALSStatistics() {
	long statDummy;
	if (IntegratorSundialsCVODE* intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(m_integrator)) {

		CVSpilsGetNumLinIters(intCVODE->cvodeMem(),&statDummy);
		m_statNumLinIters = statDummy;
		CVSpilsGetNumConvFails(intCVODE->cvodeMem(),&statDummy);
		m_statNumLinConvFails = statDummy;
		CVSpilsGetNumPrecEvals(intCVODE->cvodeMem(),&statDummy);
		m_statNumPrecEvals = statDummy;
		CVSpilsGetNumPrecSolves(intCVODE->cvodeMem(),&statDummy);
		m_statNumPrecSolves = statDummy;
		CVSpilsGetNumJtimesEvals(intCVODE->cvodeMem(),&statDummy);
		m_statNumJacTimesEvals = statDummy;
		CVSpilsGetNumRhsEvals(intCVODE->cvodeMem(),&statDummy);
		m_statNumRhsEvals = statDummy;
	}
}


void LESInterfaceIterative::writeMetrics(double simtime, std::ostream * metricsFile) {
	const char * const FUNC_ID = "[LESInterfaceIterative::writeMetrics]";
	updateSUNDIALSStatistics();

	std::string ustr = IBK::Time::suitableTimeUnit(simtime);
	double tfeval = TimerSum(SUNDIALS_TIMER_FEVAL_LS_SOLVE);
	double tFEvalJac = TimerSum(SUNDIALS_TIMER_FEVAL_JACOBIAN_GENERATION);
	double tJacAssembly = TimerSum(SUNDIALS_TIMER_JACOBIAN_GENERATION);
	double tATimes = TimerSum(SUNDIALS_TIMER_ATIMES);
	double tFEvalPreSetup = TimerSum(SUNDIALS_TIMER_FEVAL_PRE_SETUP);
	double tPreSetup = TimerSum(SUNDIALS_TIMER_PRE_SETUP);
	double tPreSolve = TimerSum(SUNDIALS_TIMER_PRE_SOLVE);
	unsigned int nFeJacSetup = 0;
	if (m_jacobian != NULL)
		nFeJacSetup = m_jacobian->nRHSEvals();
	unsigned int nFePrecSetup = 0;
	if (m_precond != NULL)
		nFePrecSetup = m_precond->nRHSEvals();

	// write metrics to screen
	IBK::IBK_Message( IBK::FormatString("LES: Linear iterations                     =                          %1\n")
		.arg(m_statNumLinIters,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("LES: Linear convergence failures           =                          %1\n")
		.arg(m_statNumLinConvFails,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("LES: Function evaluations (LS solve)       = %1 (%2 %%)\n")
		.arg(IBK::Time::format_time_difference(tfeval, ustr, true),13)
		.arg(tfeval/simtime*100, 5, 'f', 2)
		.arg(m_statNumRhsEvals,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("LES: Function evaluations (Jacobian gen.)  = %1 (%2 %%)  %3\n")
		.arg(IBK::Time::format_time_difference(tFEvalJac, ustr, true),13)
		.arg(tFEvalJac/simtime*100, 5, 'f', 2)
		.arg(nFeJacSetup,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("LES: Jacobian matrix assembly time         = %1 (%2 %%)\n")
		.arg(IBK::Time::format_time_difference(tJacAssembly, ustr, true),13)
		.arg(tJacAssembly/simtime*100, 5, 'f', 2),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("LES: Matrix-vector multiplications         = %1 (%2 %%)  %3\n")
		.arg(IBK::Time::format_time_difference(tATimes, ustr, true),13)
		.arg(tATimes/simtime*100, 5, 'f', 2)
		.arg(m_statNumJacTimesEvals,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("LES: Function evaluations (precond. setup) = %1 (%2 %%)  %3\n")
		.arg(IBK::Time::format_time_difference(tFEvalPreSetup, ustr, true),13)
		.arg(tFEvalPreSetup/simtime*100, 5, 'f', 2)
		.arg(nFePrecSetup,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("LES: Preconditioner setup                  = %1 (%2 %%)  %3\n")
		.arg(IBK::Time::format_time_difference(tPreSetup, ustr, true),13)
		.arg(tPreSetup/simtime*100, 5, 'f', 2)
		.arg(m_statNumPrecEvals,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("LES: Preconditioner solves                 = %1 (%2 %%)  %3\n")
		.arg(IBK::Time::format_time_difference(tPreSolve, ustr, true),13)
		.arg(tPreSolve/simtime*100, 5, 'f', 2)
		.arg(m_statNumPrecSolves,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	if (metricsFile != NULL) {
		*metricsFile << "LESLinearIters=" << m_statNumLinIters << std::endl;
		*metricsFile << "LESLinearConvFails=" << m_statNumLinConvFails << std::endl;
		*metricsFile << "LESRHSEvals=" << m_statNumRhsEvals << std::endl;
		*metricsFile << "LESTimeRHSEvals=" << tfeval << std::endl;
		*metricsFile << "LESRHSEvalsJacGeneration=" << nFeJacSetup << std::endl;
		*metricsFile << "LESTimeRHSEvalsJacGeneration=" << tFEvalJac << std::endl;
		*metricsFile << "LESTimeJacAssembly=" << tJacAssembly << std::endl;
		*metricsFile << "LESMatrixVectorMult=" << m_statNumJacTimesEvals << std::endl;
		*metricsFile << "LESTimeMatrixVectorMult=" << tATimes << std::endl;
		*metricsFile << "LESRHSEvalsPreSetup=" << nFePrecSetup << std::endl;
		*metricsFile << "LESTimeRHSEvalsPreSetup=" << tFEvalPreSetup << std::endl;
		*metricsFile << "LESPreSetup=" << m_statNumPrecEvals << std::endl;
		*metricsFile << "LESTimePreSetup=" << tPreSetup << std::endl;
		*metricsFile << "LESPreSolves=" << m_statNumPrecSolves << std::endl;
		*metricsFile << "LESTimePreSolves=" << tPreSolve << std::endl;
	}
}

} // namespace SOLFRA


