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

#ifndef SolverControlFrameworkH
#define SolverControlFrameworkH

#include <string>
#include <IBK_StopWatch.h>
#include <IBK_Path.h>

/*!	\brief The namespace SOLFRA encapsulates interfaces and integrators of
	the solver control framework.

	Key classes are SolverControlFramework, and the integrators
	- IntegratorExplicitEuler,
	- IntegratorImplicitEuler
	- IntegratorRungeKutta45,
	- IntegratorSundialsCVODE,
*/
namespace SOLFRA {

class ModelInterface;
class IntegratorInterface;
class OutputScheduler;
class LESInterface;
class PrecondInterface;
class JacobianInterface;

/*!	\brief Declaration for class SolverControlFramework
	\author Andreas Nicolai <andreas.nicolai -[at]- tu-dresden.de>

	This class implements the core integration loop, tells the integrator to take
	steps and processes outputs. It also handles restart functionality and other
	general integration functionality.
*/
class SolverControlFramework {
public:
	/*! Defines options for the restart file handling. */
	enum RestartFileMode {
		/*! Store only the restart snapshot at simulation end (no interim restart points). */
		RestartInfoOnlyAtSimulationEnd,
		/*! Store only last restart snapshot. */
		RestartFromLast,
		/*! Store full restart history. */
		RestartFromAll
	};

	/*! Creates an instance of the solver control framework.
		The instance of the model will NOT be owned by the solver framework
		and the memory will not be released. The user has to ensure that
		the corresponding object lives as long as the solver control framework instance
		is in use.
	*/
	SolverControlFramework(ModelInterface * model = nullptr);
	/*! Virtual destructor, so that this class can be derived and reimplemented. */
	virtual ~SolverControlFramework();

	/*! Sets/replaces references to model interface implementations.
		The instance of the model will NOT be owned by the solver framework
		and the memory will not be released. The user has to ensure that
		the corresponding object lives as long as the solver control framework instance
		is in use.
		A call to setModel() will reset integrator, les-solver and output scheduler.
	*/
	void setModel(ModelInterface * model);

	/*! Sets/replaces references to integrator interface implementations.
		The instance of the integrator will NOT be owned by the solver framework
		and the memory will not be released. The user has to ensure that
		the corresponding object lives as long as the solver control framework instance
		is in use.
		\warning A call to setModel() will reset the integrator with the model's own
		integrator. Call setIntegrator() _after_ setModel()!
	*/
	void setIntegrator(IntegratorInterface * integrator);

	/*! Sets/replaces references to linear equation system solver implementations.
		The instance of the linear equation system solver will NOT be owned by the solver framework
		and the memory will not be released. The user has to ensure that
		the corresponding object lives as long as the solver control framework instance
		is in use.
		\warning A call to setModel() will reset the linear equation system solver with the model's own
		integrator. Call setLESSolver() _after_ setModel()!
	*/
	void setLESSolver(LESInterface * lesSolver);

	/*! Sets a new preconditioner interface.
		The instance of the preconditioner will NOT be owned by the solver framework
		and the memory will not be released. The user has to ensure that
		the corresponding object lives as long as the solver control framework instance
		is in use.
		\warning A call to setModel() will reset the preconditioner with the model's own
		preconditioner. Call setPreconditioner() _after_ setModel()!
	*/
	void setPreconditioner(PrecondInterface	* precondInterface);

	/*! Sets a new jacobian interface.
		The instance of the jacobain will NOT be owned by the solver framework
		and the memory will not be released. The user has to ensure that
		the corresponding object lives as long as the solver control framework instance
		is in use.
		\warning A call to setModel() will reset the jacobian with the model's own
		jacobain. Call setJacobian() _after_ setModel()!
	*/
	void setJacobian(JacobianInterface	* jacobianInterface);

	/*! Sets a new output scheduler.
		The instance of the output scheduler will NOT be owned by the solver framework
		and the memory will not be released. The user has to ensure that
		the corresponding object lives as long as the solver control framework instance
		is in use.
		\warning A call to setModel() will reset the output scheduler with the model's own
				 output scheduler. Call setOutputScheduler() _after_ setModel()!
	*/
	void setOutputScheduler(OutputScheduler	* outputScheduler);

	/*! Specifies the restart file name. */
	void setRestartFile(const std::string & fname, RestartFileMode restartMode=RestartFromLast);

	/*! Returns pointer to integrator implementation (not owned). */
	const IntegratorInterface		*integrator() const { return m_integrator; }

	/*! Reads restart information and begins from the step at or just before t. */
	void restartFrom(double t);

	/*! Reads restart information and begins from the step with index step.
		\param step The step index to continue the simulation from.
			If 0, the simulation is started regularly from begin, same as calling run().
			If -1, the simulation is started from the last recorded restart point (this is the default).
	*/
	void restart(int step = -1);

	/*! Runs the model integrator, actually the same as restartFrom(0). */
	void run() { restart(0); }

	/*! Writes a formatted output message to notify user of integration process, whereby the
		function is called either after each successful integration step or when outputs are written.
		When you re-implement the SolverControlFramework class you can also re-implement
		this function to observe the integration process and, for example, feed a gui component with
		progress/statistics data.
	*/
	virtual void writeProgress(double t, bool output) { (void)t; (void)output; }

	/*! This function writes a summary of the solver metrics and timings obtained through the
		last call to run() or restart().
		The output is forked off to the various components of the integrator platform.
	*/
	void writeMetrics();

	/*! Print version information about the integrated solvers to screen. */
	static void printVersionInfo();
	/*! Helper function to show content of restart file. */
	static void printRestartFileInfo(const IBK::Path & restartFilePath);


	/*! Full path to log file directory, by default empty (solver executable working directory). */
	IBK::Path				m_logDirectory;

	/*! Full path to the file name of restart file.
		By default this is empty, so no restart data is being written.
		The user of the solver framework must specify the restart flag manually.
	*/
	IBK::Path				m_restartFilename;
	/*! Defines restart file handling. */
	RestartFileMode			m_restartMode;

	/*! If set to true before a call to run() or restart(), the framework will
		return from run() or restart() once the solver initialization was done.

		Setting this flag is useful to test whether the solver initialization runs through
		for a specific test case.
	*/
	bool					m_stopAfterSolverInit;
	/*! If true, we write step-by-step statistics (useful for debugging).
		If this flag is false (the default) statistics will be written just after an output was
		requested.
	*/
	bool					m_useStepStatistics;

protected:
	/*! Internal integration loop function used by restart(), restartFrom() and run().
		Throws an IBK::Exception if solver run fails.
	*/
	void run(double t0);

	/*! Re-opens restart file for writing and appends new solution at end. */
	void appendRestartInfo(double t, const double * y) const;

	/*! Reads restart file.
		\param step Can be either:
			- -1 read last solution
			- -2 use t_restart to determine start time point
		\param t_restart If step == -2, the restart file is read until t > t_restart
		\param realT_restart Real time elapsed up to this time point.
		\param t Time point of restart is stored here.
		\param integratorModelData Solution at restart time (size n + m_model->serializationSize()/sizeof(double) )
								   is stored here and any additional model data.
		\param restartFileCopy Output file stream to be used for copying the restart file data into, up to the requested point.
		\return Returns true if successful.
	*/
	bool readRestartFile(int step,
		double t_restart,
		double & t, double * integratorModelData,
		std::ostream * restartFileCopy = nullptr) const;

	/*! Pointer to model implementation (not owned). */
	ModelInterface			*m_model;
	/*! Pointer to integrator implementation (not owned). */
	IntegratorInterface		*m_integrator;
	/*! Pointer to linear equation system solver implementation (not owned). */
	LESInterface			*m_lesSolver;
	/*! Pointer to Preconditioner interface (not owned). */
	PrecondInterface		*m_precondInterface;
	/*! Pointer to Jacobian matrix interface (not owned). */
	JacobianInterface		*m_jacobianInterface;
	/*! Pointer to output scheduler implementation (not owned). */
	OutputScheduler			*m_outputScheduler;

	/*! The central stopwatch, to measure execution time. */
	IBK::StopWatch			m_stopWatch;

	/*! Pointer to default integrator implementation (owned and released). */
	IntegratorInterface		*m_defaultIntegrator;
	/*! Pointer to default linear equation system solver implementation (owned and released). */
	LESInterface			*m_defaultLES;
	/*! Pointer to default output scheduler implementation (owned and released). */
	OutputScheduler			*m_defaultOutputScheduler;
};

} // namespace SOLFRA


#endif // SolverControlFrameworkH
