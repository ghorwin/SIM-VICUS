#include "SOLFRA_SolverFeedback.h"

#include <sstream>
#include <iomanip>

#include <IBK_Time.h>
#include <IBK_messages.h>

#include "SOLFRA_ModelInterface.h"

namespace SOLFRA {


void SolverFeedback::setup(std::ostream * progressLog, double t0, double tEnd, const std::string & interimMessage,
	double elapsedSecondsAtStart, double elapsedSimTimeAtStart, SOLFRA::ModelInterface * modelInterface)
{
	m_progressLog = progressLog;
	m_progressT0 = t0;
	m_progressTEnd = tEnd;
	m_interimMessage = interimMessage;
	m_notifyTime = 15; // 15 seconds from now

	m_elapsedSecondsAtStart = elapsedSecondsAtStart;
	m_elapsedSimTimeAtStart = elapsedSimTimeAtStart;
	m_lastElapsedSeconds = m_elapsedSecondsAtStart;
	m_lastElapsedSimTime = m_elapsedSimTimeAtStart;
	m_lastElapsedSecondsWithOutput= m_elapsedSecondsAtStart;
	m_ETC = -1;

	m_modelInterface = modelInterface;
	// set counter to 41, so that next call to writeOutputs()
	// will trigger writing of header line
	m_outputCounter = 41;

	// write header
	if (progressLog != nullptr)
		*progressLog << std::setw(15) << "Simtime [s] " << '\t' << std::setw(15) << "Realtime [s]" << '\t' << std::setw(15) << "Percentage [%]" << '\n';

	m_stopWatch.start();
}


void SolverFeedback::writeFeedback(double t, bool betweenOutputs) {
    FUNCID(SolverFeedback::writeFeedback);
	//if (m_abort)  throw IBK_exception("Aborting solver...", "[ModelImpl::writeFeedback]");
	// calculate statistics

	// compute total elapsed simulation time [s]
	double elapsedSimTime = t - m_elapsedSimTimeAtStart;
	// compute total elapsed real time, including time spend in previous runs when
	// solver was continued.
	double elapsedSecondsThisStep = 0.001*m_stopWatch.difference();
	double elapsedSeconds = elapsedSecondsThisStep + m_elapsedSecondsAtStart; // in [s]
	elapsedSecondsThisStep -= m_lastElapsedSecondsWithOutput; // holds now the time since last written output

	// the average speed in [s/s]
	double averageSpeed;
	if (elapsedSeconds > 0)
		averageSpeed = elapsedSimTime/elapsedSeconds;
	else
		averageSpeed = 0;
	// add current time point to m_outputTimes
	m_outputTimes.push_back( std::make_pair(elapsedSeconds, t) );
	// use different real time range for sliding average calculation, based on  mean speed
	double ave_real_time = 600; // 10 mins as default
	// faster than 1d/s
	if (averageSpeed > 24*3600) {
		ave_real_time = 5; // 5 seconds
	}
	// faster than 2h/s
	else if (averageSpeed > 2*3600) {
		ave_real_time = 60; // one minute
	}
	// calculate sliding average over past 20 mins
	int i = m_outputTimes.size();
	double delta_t_real = 0;
	while (i > 0) {
		--i;
		delta_t_real = m_outputTimes.back().first - m_outputTimes[i].first;
		if (delta_t_real >= ave_real_time) break; // got enough real time to calculate average
	}
	// compute sliding average of computation speed
	double slidingAverageSpeed;
	if (delta_t_real == 0) {
		slidingAverageSpeed = 0;
	}
	else {
		double delta_t_sim = m_outputTimes.back().second - m_outputTimes[i].second;
		slidingAverageSpeed = delta_t_sim/delta_t_real;
	}
	// compute estimated time of completion
	if (slidingAverageSpeed < 1e-5)		m_ETC = -1;
	else                    			m_ETC = (m_progressTEnd - t)/slidingAverageSpeed;
	m_lastElapsedSimTime = t;
	m_lastElapsedSeconds = elapsedSeconds;

	// write console output

	// only write if at least half a second has passed
	if (elapsedSecondsThisStep < 0.5)
		return;

	std::stringstream strm;
	if (--m_outputCounter % 20 == 0) {
		if (!m_outputCounter) m_outputCounter = 40;
		if (m_outputCounter == 20)
			strm << m_interimMessage << '\n';
		else
			strm << "Simtime     Simdate           Realtime    MeanSpeed    CurrentSpeed ETC\n";
	}
	std::string dateTimeStr = m_modelInterface->simTime2DateTimeString(t);
	strm << std::setw(11) << std::left << IBK::Time::format_time_difference(t) << " "
			<< dateTimeStr << " "
			<< std::setw(11) << std::left << IBK::Time::format_time_difference(elapsedSeconds) << " "
			<< std::setw(12) << std::left << (IBK::Time::format_time_difference(averageSpeed)+"/s") << " "
			<< std::setw(12) << std::left << (IBK::Time::format_time_difference(slidingAverageSpeed)+"/s") << " "
			<< ((m_ETC == -1.0) ? std::string("---") : IBK::Time::format_time_difference(m_ETC))
			<< (betweenOutputs ? " *" : "") << "\n";
	IBK::IBK_Message( strm.str(), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	// write progress log
	if (m_progressLog != nullptr && !betweenOutputs && elapsedSimTime != 0.0) {
		*m_progressLog << std::setprecision(14) << std::setw(15) << std::right << t << '\t'
					   << std::setprecision(7) << std::setw(15) << std::right << elapsedSeconds << '\t'
					   << std::setprecision(5) << std::setw(15) << std::right << t*100/m_progressTEnd << std::endl;
	}

	m_notifyTime = 0.001*m_stopWatch.difference() + 15; // Notify again in 15 seconds
	m_lastElapsedSecondsWithOutput = elapsedSeconds;
}
// ---------------------------------------------------------------------------


// This feedback function is triggered from the solver system function calculateDivergences()
void SolverFeedback::writeFeedbackFromF(double t) {
	double elapsedSeconds = m_stopWatch.difference()*0.001; // *0.001 to convert from ms in s
	if (elapsedSeconds < m_notifyTime) return;
	//if (m_abort)  throw IBK_exception("Aborting solver...", "[SolverFeedback::writeFeedbackFromF]");

	// set next notification time point
	m_notifyTime += 15; // Every 15 seconds

	writeFeedback(t, true);
}
// ---------------------------------------------------------------------------


} // namespace SOLFRA

