#ifndef SOLFRA_IntegratorSundialsCVODEH
#define SOLFRA_IntegratorSundialsCVODEH

#include <string>
#include <vector>

#include "SOLFRA_IntegratorErrorControlled.h"

namespace SOLFRA {

class IntegratorSundialsCVODEImpl;

/*! \brief Wrapper class for CVODE integrator.

	This class encapsulates everything that is specific to the CVODE integrator. It
	hides the implementation in the private implementation class IntegratorSundialsCVODEImpl.

	The CVODE-specific parameters are public variables of this class and can be
	adjusted after creating the instance of the integrator, but _before_ calling
	the init() function.
*/
class IntegratorSundialsCVODE : public IntegratorErrorControlled {
public:
	virtual const char * identifier() const override { return "CVODE"; }

	/*! Constructor. */
	IntegratorSundialsCVODE();
	/*! Destructor. */
	virtual ~IntegratorSundialsCVODE() override;

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

	/*! Specifies the frequency for calling the linear solver setup function to
		recompute the Jacobian matrix and/or preconditioner.
		Set 0 to restore default.
		\note This function just relays the call to CVodeSetLSetupFrequency().
	*/
	void setLinearSetupFrequency(int msbp);

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
	virtual double t() const override;

	/*! Returns the time step in [s] that was used in the last step.
		Time step size is updated in a call to step().
	*/
	virtual double dt() const override;

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

	/*! Returns memory to CVODE solver for direct access to SUNDIALS API.
		The pointer will be initialized after a successful call to init(),
		otherwise it is 0.
	*/
	void * cvodeMem() const;

	/*! Solver statistics specific to the CVODE integrator. */
	enum StatisticsType {
		StatNumSteps,
		StatNumNIters,
		StatNumNCFails,
		StatNumNErrFails,
		StatNumRHSEvals,
		StatNumDlsJacEvals,
		StatNumDlsRHSEvals,
		StatNumLinIters,
		StatNumLinCFails,
		StatNumLinPrecEvals,
		StatNumLinPrecSolves,
		StatNumLinRHSEvals,
		StatNumLinJtimesEvals
	};

	/*! Returns the content of a counter variable.
		\warning Not all variables are defined for all combinations of LES solvers and
				 preconditioners.
		\param statType A statistics type.
		\sa StatisticsType
	*/
	unsigned int stats(StatisticsType statType) const;


	/*! Computes and returns serialization size, by default returns  returns an invalid value (-1). */
	virtual std::size_t serializationSize() const override;

	/*! Stores content at memory location pointed to by dataPtr and increases
		pointer afterwards to point just behind the memory occupied by the copied data.
		Default implementation does nothing.
	*/
	virtual void serialize(void* & dataPtr) const override;

	/*! Restores content from memory at location pointed to by dataPtr and increases
		pointer afterwards to point just behind the memory occupied by the copied data.
		Default implementation does nothing.
	*/
	virtual void deserialize(void* & dataPtr) override;


	// CVODE specific parameters, you can change these parameters after
	// creating an instance of the integrator, but before you call init().
	double			m_dtMin;
	double			m_dtMax;
	unsigned int	m_maxOrder;
	unsigned int	m_maxSteps;
	/*! Maximum number of corrector attempts (maxcor) in Newton iteration (0 = use CVODE default). */
	unsigned int	m_maxNonLinIters;
	/*! Safety factor for residual control in Newton iteration ((0 = use CVODE default). */
	double			m_nonLinConvCoeff;
	/*! If set to true, the CVODE integrator will be initialized using the BDF stability limit detection flag (default is false). */
	bool			m_stabilityLimitDetectionEnabled;

private:
	/*! Implementation class. */
	IntegratorSundialsCVODEImpl		*m_impl;

	friend class IntegratorSundialsCVODEImpl;
};

} // namespace SOLFRA

#endif // SOLFRA_IntegratorSundialsCVODEH
