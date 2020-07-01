#ifndef SOLFRA_SolverFeedbackH
#define SOLFRA_SolverFeedbackH

#include <vector>
#include <string>
#include <iostream>

#include <IBK_StopWatch.h>



namespace SOLFRA {

class ModelInterface;

/*! Class that provides member variables and feedback implementation for
	writing solver feedback.

	Use private inheritance or aggregation and call feedback() and feedbackFromF() in
	your code after an initial call to setup().
*/
class SolverFeedback {
public:
	/*! Setup of the feedback class.
		\param progressLog		Stream to a log file containing realtime, simtime, and gliding average (NULL to disable log file).
		\param t0 Simulation starting time point in [s].
		\param tEnd Simulation ending time point in [s].
		\param interimMessage	Message to show in regular intervals instead of header captions, for example
								project file name.
		\param elapsedSecondsAtStart Realtime seconds elapsed after start of simulation (used for restarting).
		\param elapsedSimTimeAtStart Simulation time seconds elapsed after start of simulation (used for restarting).
		\param modelInterface Pointer to model interface, used to convert (relative) simtime into a date/time string.
	*/
	void setup(std::ostream * progressLog, double t0, double tEnd, const std::string & interimMessage,
		double elapsedSecondsAtStart, double elapsedSimTimeAtStart,
		SOLFRA::ModelInterface * modelInterface);

	/*! Writes feedback to the user (notification call).
		This function is typically called from Model::writeOutputs().
	*/
	void writeFeedback(double t, bool betweenOutputs);
	/*! Writes feedback to user.
		This function is called from the ydot-function or residuals-function, in case
		output time points are too far apart. It essentially writes the same
		information as writeFeedback, but with some extra stuff added. */
	void writeFeedbackFromF(double t);

	/*! Message to show in regular intervals instead of header captions, for example
		project file name.
	*/
	std::string								m_interimMessage;
	/*! Simulation starting time point in [s]. */
	double									m_progressT0;
	/*! Simulation ending time point in [s]. */
	double									m_progressTEnd;

	/*! Log file containing realtime, simtime, and gliding average. */
	std::ostream							*m_progressLog;
	/*! Keeps time elapsed since solver start/restart. */
	IBK::StopWatch							m_stopWatch;
	/*! Holds seconds elapsed in last run in case solver was continued.
		This value is normally = 0, except when the solver was restarted. Then, the
		time needed in the previous run is stored in this variable.
	*/
	double									m_elapsedSecondsAtStart;
	/*! Holds simulation time in [s] already elapsed at first call of the model
		(sim time already computed when the solver was restarted).
		This value is normally = t0(), except when the solver was restarted. Then, the
		simulation time reached in the previous run is stored in this variable.
	*/
	double									m_elapsedSimTimeAtStart;
	/*! Next elapsed time (see m_stopWatch) to write notification in-between outputs. */
	double									m_notifyTime;
	/*! Elapsed time (see m_stopWatch) at last notification. */
	double									m_lastElapsedSeconds;
	/*! Elapsed time (see m_stopWatch) at last notification with written output. */
	double									m_lastElapsedSecondsWithOutput;
	/*! Simulation time at last notification. */
	double									m_lastElapsedSimTime;
	/*! Estimated time of completion (ETC), seconds left until simulation is done. */
	double									m_ETC;
	/*! Number of outputs done since header line was last printed. */
	unsigned int							m_outputCounter;
	/*! Output time points for the calculation of sliding average of speed. */
	std::vector< std::pair<double,double> > m_outputTimes;

	/*! Pointer to the model interface. */
	SOLFRA::ModelInterface					*m_modelInterface;
};


} // namespace SOLFRA

#endif // SOLFRA_SolverFeedbackH

