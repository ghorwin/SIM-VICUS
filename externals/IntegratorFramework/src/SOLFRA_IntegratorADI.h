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

#ifndef IntegratorADIH
#define IntegratorADIH

#include <string>
#include <vector>

#include "SOLFRA_IntegratorErrorControlled.h"
#include "SOLFRA_ModelInterfaceADI.h"

// enable some additional debug output
//#define SOLVER_DEBUG_OUTPUT


namespace SOLFRA {

class LESInterface;
class PrecondInterface;
class LESADIDirect;

/*!	\brief Declaration for class IntegratorADI

	\todo Anne: Merge LESADIDirect to LESADI
	\todo Anne: initialize member variables of IntegratorADI properly in init().
*/
class IntegratorADI : public IntegratorErrorControlled {
public:
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

	virtual const char * identifier() const override { return "ADI"; }

	/*! Default constructor.
		\param dt Time step size in [s], if 0 (the default) the time step is
				  queried from the model.
	*/
	IntegratorADI();

	/*! Release allocated resources (statistics file stream etc). */
	virtual ~IntegratorADI();

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
	virtual double dtJacUpdateX() const { return m_dtJacUpdateX; }

	/*! Returns the time step in [s] that was used in the last jacobian update.
	*/
	virtual double dtJacUpdateY() const { return m_dtJacUpdateY; }

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

	/*! Returns linear equation system solver. */
	LESInterface * lesInterface() { return (LESInterface*) m_lesSolver;}

	// *** SOLVER PARAMETERS ***

	/*! Maximum number of non-linear iterations. */
	unsigned int				m_maximumNonlinearIterations;
	/*! Tolerance comparison to use in convergence test in Newton method. */
	double						m_NLResidualTolerance;
	/*! Safety factor to apply to Delta norm in Newton convergence test. */
	double						m_nonLinConvCoeff;
	/*! If true, each Newton iteration will be started with a Jacobian update. */
	ModifiedNewtonStrategies	m_modifiedNewtonStrategy;

	/*! Maximum allowed time step size. */
	double		m_dtMax;

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

	/*! Attempts to take a non-linear step.
		This function attempts to solve an integration step starting from current solution in m_y
		and current time point m_t, using time step size m_dt, thus solving the equation system for
		time point m_t + m_dt.
		Once completed, the statistic variables contain the number of needed iterations, convergence rate, etc.
		Also, m_y, m_ydot and m_F will hold the computed solution at the new time point. m_t and m_dt are _not_
		modified.
		\return Returns a flag (a enumeration value of type StepResult).
	*/
	StepResult tryStep();

	/*! Reduces time step size in m_dt based on the most recent failure of the non-linear iteration method. */
	void decreaseTimeStepNonLinearFailure();

	/*! Performs the Newton-iteration */
	StepResult newtonIteration();

	/*! Checks return code from setTime(), setY() and ydot() functions and throws IBK::Exceptions in all
		cases where a simulation abort is appropriate. */
	void handleErrors(ModelInterface::CalculationResult res) const;

	/*! Computes and returns Euclidean norm of vector vec (size m_n), which is first normalized using a
		norm calculated based on the weights stored in m_weights vector.
	*/
	double WRMSNorm(std::vector<double> & vec) const;


	/*! Pointer to model. */
	ModelInterfaceADI	*m_modelADI;

	/*! Pointer to linear equation system solver. */
	LESADIDirect		*m_lesSolver;

	/*! Pointer to Preconditioner. */
	PrecondInterface	*m_preconditioner;

	/*! Cached system size. */
	unsigned int m_n;

	/*! Current time point in [s]. */
	double m_t;
	/*! Constant time step size in [s] used in the last step. */
	double m_dt;
	/*! Constant time step size in [s] of the previously accepted time step. */
	double m_dtn;
	/*! Time step size in [s] to be used in the next call to step as first attempt. */
	double m_dtNext;


	/*! States at time point m_t. */
	std::vector<double> m_y;
	/*! States at time point m_t - 0.5 m_dt. */
	std::vector<double> m_ynHalf;
	/*! States at time point m_t - m_dt. */
	std::vector<double> m_yn;
	/*! Time derivatives at time point m_t. */
	std::vector<double> m_ydot;
	/*! Time derivatives, x-step. */
	std::vector<double> m_ydotX;
	/*! Time derivatives, x-step. */
	std::vector<double> m_ydotY;
	/*! Time derivatives at the last time point. */
	std::vector<double> m_ydotn;
	/*! Time derivatives for x-step at point m_t - 0.5 m_dt. */
	std::vector<double> m_ydotXnHalf;
	/*! Time derivatives for y-step at point m_t - _dt. */
	std::vector<double> m_ydotYn;
	/*! Weights for residual/delta norm calculation, updated whenever m_y changes.
		By default, each m_y value is multiplied with reltol, then abstol is added,
		and the inverse value is stored in the weights vector, so that the multiplication
		with a residual/tolerance will give a value that can be compared with 1.
	*/
	std::vector<double> m_weights;
	/*! Residuals of balance equations at time point m_t. */
	std::vector<double> m_residuals;
	/*! Residuals of of Crank Nicholson integrator for the time point m_t. */
	std::vector<double> m_splittingErrors;
	/*! Difference of Crank Nicholson solution to Implicit Euler solution time point m_t. */
	std::vector<double> m_residualErrors;
	/*! Difference of Crank Nicholson solution to Implicit Euler solution at the last time point. */
	std::vector<double> m_residualErrorsn;
	/*! Global error vector for the Crank Nicolson interpolation time point m_t. */
	std::vector<double> m_errors;

	/*! Differences in states from current iteration. */
	std::vector<double> m_deltaY;

	/*! Stores number of iterations used in Newton step
		(counter is increased in non-linear iteration loop until m_nMaxIterations is reached). */
	unsigned int m_nIterations;
	/*! Holds number of iterations since last update of Jacobian for x-direction step. */
	unsigned int m_nItersSinceJacUpdateX;
	/*! Holds number of iterations since last update of Jacobian for y-direction step. */
	unsigned int m_nItersSinceJacUpdateY;
	/*! Holds number of steps since last update of Jacobian for x-direction step. */
	unsigned int m_nStepsSinceJacUpdateX;
	/*! Holds number of steps since last update of Jacobian for y-direction step. */
	unsigned int m_nStepsSinceJacUpdateY;
	/*! Holds time step used when computing Jacobian for x-direction step. */
	double m_dtJacUpdateX;
	/*! Holds time step used when computing Jacobian for y-direction step. */
	double m_dtJacUpdateY;
	/*! Holds the current/estimated convergence rate based on the delta norm for the current Jacobian.*/
	double	m_convRateDeltaNormX;
	/*! Holds the current/estimated convergence rate based on the delta norm for the current Jacobian.*/
	double	m_convRateDeltaNormY;
	/*! Stores convergence rate based on vector norm of residuals of the balance equation. */
	double m_convRateResiduals;
	/*! Stores convergence rate based on vector norm of deltaY from current iteration step. */
	double m_convRateDelta;

	/*! Set to true if the last call to tryStep() signaled TryAgain with outdated Jacobian, x-step. */
	bool m_newtonConvergenceErrorX;
	/*! Set to true if the last call to tryStep() signaled TryAgain with outdated Jacobian, y-step. */
	bool m_newtonConvergenceErrorY;
	/*! If true, the Jacobian matrix will be updated before a non-linear iteration for x-step. */
	bool m_jacUpdateNeededX;
	/*! If true, the Jacobian matrix will be updated before a non-linear iteration for y-step. */
	bool m_jacUpdateNeededY;
	/*! If true, the Jacobian matrix was updated in the previous call to tryStep(). */
	bool m_jacCurrentX;
	/*! If true, the Jacobian matrix was updated in the previous call to tryStep(). */
	bool m_jacCurrentY;

	/*! States interpolated in function y_out. */
	mutable std::vector<double> m_yout;

	/*! Statistics file stream. */
	std::ostream *m_statsFileStream;

	/*! Residual norm matching the current residuals in m_residuals. */
	double m_residualNorm;
	/*! Stores the truncation error of one time step. */
	double m_truncationError;
	/*! Stores the splitting error of one time step. */
	double m_splittingError;
	/*! Stores the error of the one CN step compared to implicit Euler. */
	double m_residualError;
	/*! Maximum factor by which time step can be enlarged after end of successful step. */
	double m_dtScaleMax;

	/*! Stores total number of steps. */
	unsigned int m_statNumSteps;
	/*! Stores number of RHS (system function) evaluations done. */
	unsigned int m_statNumRHSEvals;
	/*! Total number of iterations for x-step. */
	unsigned int m_statNumItersX;
	/*! Total number of iterations for y-step. */
	unsigned int m_statNumItersY;
	/*! Total number of convergence failures for x-step. */
	unsigned int m_statNumConvFailsX;
	/*! Total number of convergence failures for y-step. */
	unsigned int m_statNumConvFailsY;
	/*! Number of diverging iterations for x-step. */
	unsigned int m_statNumConvDivergingX;
	/*! Number of diverging iterations for y-step. */
	unsigned int m_statNumConvDivergingY;
	/*! Number of Jacobian matrix evaluations and LU factorizations for x-step. */
	unsigned int m_statNumJacEvalsX;
	/*! Number of Jacobian matrix evaluations and LU factorizations for y-step. */
	unsigned int m_statNumJacEvalsY;
	/*! Total number of error test failures. */
	unsigned int m_statNumErrFails;

#ifdef SOLVER_DEBUG_OUTPUT
	std::string m_logFilePath;
#endif

};

} // namespace SOLFRA

#endif // IntegratorADIH
