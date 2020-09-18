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

#ifndef IntegratorImplicitEulerH
#define IntegratorImplicitEulerH

#include <string>
#include <vector>

#include <IBK_StopWatch.h>

#include "SOLFRA_IntegratorErrorControlled.h"
#include "SOLFRA_ModelInterface.h"

// some additional output for debugging
//#define SOLVER_DEBUG_OUTPUT

namespace SOLFRA {

class LESInterface;
class PrecondInterface;

/*!	\brief Declaration for class IntegratorImplicitEuler

	This is a state-of-the-art implementation of an implicit Euler
	method, using Newton-Raphson iteration, customizable matrix
	structures and finite-difference approximation of the Jacobian
	matrix.
*/
class IntegratorImplicitEuler : public IntegratorErrorControlled {
public:
	virtual const char * identifier() const { return "Implicit Euler (ODE)"; }

	/*! The various modified Newton strategies available for the Implicit Euler. */
	enum ModifiedNewtonStrategies {
		/*! Classical Modified Newton a la CVODE, Jacobian is updated every few steps, but only at the beginning */
		MN_ONCE_AT_STEP_START,
		/*! Modified Newton, but update is enforced at begin of every step. */
		MN_ONCE_EVERY_STEP,
		/*! Almost strict Newton, but update is stopped once residual dropped enough. */
		MN_EVERY_ITERATION_UNTIL_THRESHOLD,
		/*! Strict Newton, Jacobian matrix is updated before every iteration. */
		MN_EVERY_ITERATION
	};

	/*! Default constructor.
		\param dt Time step size in [s], if 0 (the default) the time step is
				  queried from the model.
	*/
	IntegratorImplicitEuler();

	/*! Release allocated resources (statistics file stream etc). */
	virtual ~IntegratorImplicitEuler() override;

	/*! Initializes the Integrator.
		\param model The physical model instance.
		\param lesSolver Linear equation system solver, can be nullptr if (explicit) solver doesn't need LES solver functionality.
		\param precond Preconditioner, in case that an iterative linear equation system solver is used.
			If nullptr, no preconditioner is specified.
		\param t0 Starting time point.
		\param y0 Pointer to linear memory array of size model->n() holding the initial conditions.
	*/
	virtual void init(ModelInterface * model, double t0, const double * y0,
					  LESInterface * lesSolver,
					  PrecondInterface * precond,
					  JacobianInterface * jacobian) override;

	/*! Advances the solution from the current time to the next, thereby
		adjusting the time step.
		If the step was completed sucessfully, the functions t() and dt() can be
		used to query the new time point and the step size just used, respectively.
		\return Returns an enum value indicating the outcome of the step. For
				negative values, the state of the solver is undetermined.
	*/
	virtual IntegratorInterface::StepResultType step() override;

	/*! Returns the current time point in simulation time in [s].
		Current time point is updated in a call to step().
	*/
	virtual double t() const override { return m_t; }

	/*! Returns the time step in [s] that was used in the last step.
		Time step size is updated in a call to step().
	*/
	virtual double dt() const override { return m_dt; }

	/*! Returns the time step in [s] that was used in the last jacobian update.
	*/
	virtual double dtJacUpdate() const { return m_dtJacUpdate; }

	/*! Returns a pointer to the memory array with interpolated states at the
		given output time.
		\param t_out A time point between the current time point and the
					 previous time point.
		\return A constant pointer to the memory array holding the interpolated data.
		\warning The lifetime of the pointer returned from this function may be limited.
				 At latest after a call to step() or another y_out() call, the pointer
				 is invalidated and has to be queried again.
	*/
	virtual const double * yOut(double t_out) const override;

	/*! Called from the framework to write create statistics file and write its header. */
	virtual void writeStatisticsHeader(const IBK::Path & logfilePath, bool doRestart) override;

	/*! Writes currently collected statistics. */
	virtual void writeStatistics() override;

	/*! Writes currently collected solver metrics/statistics to output.
		\param simtime Totel elapsed wall clock time of simulation in [s] (needed for percentage calculation)
		\param metricsFile If not nullptr, computer-readible metrics are written to the file.
	*/
	virtual void writeMetrics(double simtime, std::ostream * metricsFile=nullptr) override;


	// *** SOLVER PARAMETERS ***

	/*! Maximum number of non-linear iterations. */
	unsigned int				m_maxNonLinIters;
	/*! Tolerance comparison to use in convergence test in Newton method. */
	double						m_NLResidualTolerance;
	/*! Safety factor to apply to Delta norm in Newton convergence test. */
	double						m_nonLinConvCoeff;
	/*! If true, each Newton iteration will be started with a Jacobian update. */
	ModifiedNewtonStrategies	m_modifiedNewtonStrategy;

	/*! Maximum allowed time step size. */
	double						m_dtMax;

	/*! Stores total number of steps. */
	unsigned int m_statNumSteps;
	/*! Stores number of RHS (system function) evaluations done. */
	unsigned int m_statNumRHSEvals;
	/*! Total number of iterations. */
	unsigned int m_statNumIters;
	/*! Total number of convergence failures. */
	unsigned int m_statNumConvFails;
	/*! Total number of error test failures. */
	unsigned int m_statNumErrFails;
	/*! Number of diverging iterations. */
	unsigned int m_statNumConvDiverging;
	/*! Number of Jacobian matrix evaluations and LU factorizations. */
	unsigned int m_statNumJacEvals;

private:
	/*! Return values of the tryStep() function. */
	enum StepResult {
		/*! Return value for successful return (converged within acceptable number of iterations). */
		Success,
		/*! Returned to signal that step should be retried with updated Jacobian from predicted solution. */
		TryAgain,
		/*! Solution diverged. */
		FailureDiverged,
		/*! Iteration limit for non-linear iterations exceeded. */
		FailureIterationLimitExceeded,
		/*! Linear equation system could not be solved (either singularity problem, number of linear iterations exceeded, etc.). */
		FailureEquationSystemError,
		/*! System function signaled that time step was too large (physical values out of bounds). */
		FailureSystemFunctionTimeStepTooLarge,
		/*! System function signaled a critical error and requires solver abort. */
		FailureSystemFunctionCriticalError
	};

	/*! Helper function that calls setY(), ydot() in the model using the current m_y and stores in m_ydot,
		and evaluates residuals and residual norm afterwards.
	*/
	void evaluateModel();

	/*! Perform error test based on implicit Euler solution and explicit Euler solution.
		If test is accepted and sets a new time step size in m_dtNext.
		If test is failed, m_dtNext is set to a reduced time step that would - if the solution were linear
		(and local truncation error were linear in time) - exactly match the accepted error.
	*/
	bool errorTest();

	/*! This function adjusts the time step based on the error norm of the last error test.
	*/
	void adjustTimeStep();

	/*! Evaluates various criteria and sets m_jacUpdateNeeded flag. */
	void determineJacobianUpdate();

	/*! Performs the Newton-iteration */
	StepResult newtonIteration();

	/*! Attempts to take a non-linear step.
		This function attempts to solve an integration step starting from current solution in m_y
		and current time point m_t, using time step size m_dt, thus solving the equation system for
		time point m_t + m_dt.
		Once completed, the statistic variables contain the number of needed iterations, convergence rate, etc.
		Also, m_y, m_ydot will hold the computed solution at the new time point. m_t and m_dt are _not_
		modified.
		\return Returns a flag (a enumeration value of type StepResult).
	*/
	StepResult tryStep();

	/*! Reduces time step size in m_dt based on the most recent failure of the non-linear iteration method. */
	void decreaseTimeStepNonLinearFailure();

	/*! Checks return code from setTime(), setY() and ydot() functions and throws IBK::Exceptions in all
		cases where a simulation abort is appropriate. */
	void handleErrors(ModelInterface::CalculationResult res) const;

	/*! Computes and returns Euclidean norm of vector vec (size m_n), which is first normalized using a
		norm calculated based on the weights stored in m_weights vector.
	*/
	double WRMSNorm(std::vector<double> & vec) const;

	/*! Dumps iteration statistics to IBK::IBK_Message(). */
	void writeIterationStats(double convRateResiduals, double deltaNorm, double convRateDeltaNorm) const;

	/*! Pointer to ODE model. */
	ModelInterface		*m_model;

	/*! Pointer to linear equation system solver. */
	LESInterface		*m_lesSolver;

	/*! Pointer to Preconditioner. */
	PrecondInterface	*m_preconditioner;

	/*! Cached system size. */
	unsigned int		m_n;

	/*! Current time point in [s]. */
	double				m_t;
	/*! Time step size in [s] used in the last step. */
	double				m_dt;
	/*! Time step size in [s] to be used in the next call to step() as first attempt. */
	double				m_dtNext;
	/*! Time step size in [s] used when last computing Jacobian. */
	double				m_dtJacUpdate;
	/*! Error norm last computed in doErrorTest(). */
	double				m_errNorm;
	/*! Maximum factor by which time step can be enlarged after end of successful step. */
	double				m_dtScaleMax;

	/*! If true, the Jacobian matrix will be updated before a non-linear iteration. */
	bool				m_jacUpdateNeeded;
	/*! If true, the Jacobian matrix was updated in the previous call to tryStep(). */
	bool				m_jacCurrent;
	/*! Holds the current/estimated convergence rate based on the delta norm for the current Jacobian.
		Convergence rate depends on the quality of the Jacobian matrix. Whenever the Jacobian
		matrix is updated, the convergence rate estimate is set to 1. During Newton iterations the
		convergence rate (ratio between current and last delta norm) is monitored and stored in
		m_convRateDeltaNorm. For non-linear Newton iterations the convergence rate should increase
		with each iteration by about an order of magnitude. The convergence rate obtained in the successful
		Newton step will be kept and used as starting estimate in the next Newton iteration if
		the Jacobian matrix is kept.
		To avoid excessive oszillations of the convergence rate, the value of m_convRateDeltaNorm can
		only be decreased at max by a certain factor in each iteration. It can, however, be enlarged if
		the current convergence rate proves to be slower than a previous convergence rate.

		The convergence rate estimate is used to evaluate, whether it is meaningful to do another Newton step or
		if the last step already provided a good solution. Meaning, if the current delta norm is already small,
		and the convergence rate estimate says that one more iteration (with the current solution) will get an
		almost unchanged result (= very small delta norm), the next step is not even done and the iteration
		returns already with the current solution. This procedure effectively reduces number of Newton iterations
		by a factor of 2, when the first Newton iteration already approaches the solution (small enough delta norm).
	*/
	double				m_convRateDeltaNorm;

	/*! Set to true if the last call to tryStep() signaled TryAgain with outdated Jacobian. */
	bool				m_newtonConvergenceError;

	/*! States at time point m_t.
		Within step() m_y holds iterative values at the new time point m_t + m_dt.
	*/
	std::vector<double> m_y;
	/*! Time derivatives at time point m_t.
		Within step() m_ydot holds iterative values at the new time point m_t + m_dt.
	*/
	std::vector<double> m_ydot;
	/*! Weights for residual/delta norm calculation, updated whenever m_y changes.
		By default, each m_y value is multiplied with reltol, then abstol is added,
		and the inverse value is stored in the weights vector, so that the multiplication
		with a residual/tolerance will give a value that can be compared with 1.
	*/
	std::vector<double> m_weights;
	/*! Local truncation errors as estimated from Taylor-Series expansion. */
	std::vector<double> m_errors;

	/*! States at time point m_t (used within step()). */
	std::vector<double> m_yn;
	/*! Predicted states at time point m_t + m_dt (used for error test). */
	std::vector<double> m_yPred;
	/*! Time derivatives at time point m_t. */
	std::vector<double> m_ydotn;
	/*! Residuals of balance equations at time point m_t. */
	std::vector<double> m_residuals;
	/*! Residual norm matching the current residuals in m_residuals. */
	double				m_residualNorm;

	/*! Differences in states from current iteration. */
	std::vector<double> m_deltaY;


	/*! Stores number of iterations used in Newton step
		(counter is increased in non-linear iteration loop until m_nMaxIterations is reached). */
	unsigned int m_nIterations;
	/*! Holds number of iterations since last update of Jacobian. */
	unsigned int m_nItersSinceJacUpdate;
	/*! Holds number of steps since last update of Jacobian. */
	unsigned int m_nStepsSinceJacUpdate;

	/*! States interpolated in function y_out. */
	mutable std::vector<double> m_yout;

	/*! Statistics file stream. */
	std::ostream *m_statsFileStream;

	/*! Stores the path to the log files. */
	IBK::Path m_logFilePath;

	/*! A stopwatch used to collect timings. */
	IBK::StopWatch m_watch;
};

} // namespace SOLFRA

#endif // IntegratorImplicitEulerH
