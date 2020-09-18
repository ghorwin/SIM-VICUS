#include "SOLFRA_LESInterfaceDirect.h"

#include <iomanip>
#include <iostream>
#include <fstream>

#include <cvode/cvode_direct.h>
#include <cvode/cvode.h>
#include <sundials/sundials_timer.h>

#include <IBK_assert.h>
#include <IBK_Time.h>
#include <IBK_messages.h>

#include "SOLFRA_IntegratorSundialsCVODE.h"
#include "SOLFRA_LESKLU.h"

namespace SOLFRA {

void LESInterfaceDirect::writeStatisticsHeader(const IBK::Path & logfilePath, bool doRestart) {

	if (doRestart) {
		m_statsFileStream = new std::ofstream( (logfilePath + "/LES_direct_stats.tsv").c_str(), std::ios_base::app);
	}
	else {
		m_statsFileStream = new std::ofstream( (logfilePath + "/LES_direct_stats.tsv").c_str());
		std::ostream & out = *m_statsFileStream;
		out << std::setw(25) << std::right << "Time\t";
		out << std::setw(13) << std::right << "NJacEvals\t";
		out << std::setw(13) << std::right << "NRhsEvals";
		out << std::endl;
	}
}

void LESInterfaceDirect::writeStatistics(double t) {
	updateSUNDIALSStatistics();

	// append to output file
	std::ostream & out = *m_statsFileStream;

	// Time after last step was completed
	out << std::fixed << std::setprecision(10) << std::setw(25) << std::right << t << "\t";
	// NJacEvals
	out << std::fixed << std::setprecision(0) << std::setw(13) << std::right << m_statNumJacEvals;
	// NRhsEvals
	out << std::fixed << std::setprecision(0) << std::setw(13) << std::right << m_statNumRhsEvals;
	out << std::endl;
}

void LESInterfaceDirect::updateSUNDIALSStatistics() {
	// depending on integrator, write different stats
	if (IntegratorSundialsCVODE* intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(m_integrator)) {
		long statDummy;
		// for sparse solver LESKLU the function updateSUNDIALSStatistics() is overridden
		IBK_ASSERT(dynamic_cast<LESKLU*>(this) == nullptr);
		CVDlsGetNumJacEvals(intCVODE->cvodeMem(),&statDummy);
		m_statNumJacEvals = statDummy;
		CVDlsGetNumRhsEvals(intCVODE->cvodeMem(),&statDummy);
		m_statNumRhsEvals = statDummy;
	}
}

void LESInterfaceDirect::writeMetrics(double simtime, std::ostream * metricsFile) {
	FUNCID(LESInterfaceDirect::writeMetrics);
	updateSUNDIALSStatistics();

	std::string ustr = IBK::Time::suitableTimeUnit(simtime);
	double tfactor = TimerSum(SUNDIALS_TIMER_JACOBIAN_FACTORIZATION);
	double tfeval = TimerSum(SUNDIALS_TIMER_FEVAL_JACOBIAN_GENERATION);
	unsigned int nLinSetups = 0;
	// determine which integrator we are using
	if (IntegratorSundialsCVODE* intCVODE = dynamic_cast<IntegratorSundialsCVODE*>(m_integrator)) {

		long statDummy;
		CVodeGetNumLinSolvSetups(intCVODE->cvodeMem(), &statDummy);
		nLinSetups = (unsigned int)statDummy;

	}
	else {
		/// \todo Implement for Implicit Euler
	}

	// write metrics to screen
	IBK::IBK_Message( IBK::FormatString("LES: Jacobian matrix evaluations           =                          %1\n")
		.arg(m_statNumJacEvals,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("LES: Matrix factorization                  = %1 (%2 %%)  %3\n")
		.arg(IBK::Time::format_time_difference(tfactor, ustr, true),13)
		.arg(tfactor/simtime*100, 5, 'f', 2)
		.arg(nLinSetups,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	IBK::IBK_Message( IBK::FormatString("LES: Function evaluation (Jacobian gen.)   = %1 (%2 %%)  %3\n")
		.arg(IBK::Time::format_time_difference(tfeval, ustr, true),13)
		.arg(tfeval/simtime*100, 5, 'f', 2)
		.arg(m_statNumRhsEvals,8),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	if (metricsFile != nullptr) {
		*metricsFile << "LESSetups=" << nLinSetups << std::endl;
		*metricsFile << "LESJacEvals=" << m_statNumJacEvals << std::endl;
		*metricsFile << "LESTimeJacEvals=" << tfactor << std::endl;
		*metricsFile << "LESRHSEvals=" << m_statNumRhsEvals << std::endl;
		*metricsFile << "LESTimeRHSEvals=" << tfeval << std::endl;
	}
}


} // namespace SOLFRA

