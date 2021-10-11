#ifndef SOLFRA_LESInterfaceH
#define SOLFRA_LESInterfaceH

#include "SOLFRA_Constants.h"

#include <iostream> // needed for delete operator in destructor

#include <IBK_Path.h>

namespace SOLFRA {

class ModelInterface;
class IntegratorInterface;
class PrecondInterface;
class JacobianInterface;

/*! This class defines the interface functions for a linear equation system solver.

	Special version for native sundials solver:
	- prefixed Sundials
	- re-implemented class type SundialsLinearEquationSolver provides type for
	  each native solver and provides required parameters,
	- functionality defined in LinearEquationSolverInterface is not implemented
	  in these cases

	Regular LES Implementations:
	- implement init() functions
	- implement setup() and solve(), may use model interface to let model compute
	  values, for example DQ

	  Provided default implementations:
	  Direct solvers:
	  - LESDense
	  - LESBand
	  - LESBTridiag

	  Iterative solvers:
	  - LESGMRES
	  - LESTFQMR
	  - LESBiCGStab
*/
class LESInterface {
public:

	/*! Default constructor */
	LESInterface():
		m_statNumRhsEvals(0),
		m_integrator(nullptr),
		m_model(nullptr),
		m_statsFileStream(nullptr),
		m_relToleranceDQ(1e-7),
		m_absToleranceDQ(1e-8)
	{
	}


	/*! Virtual destructor.
		Classes re-implementing the LESInterface should use
		the destructor to release allocated memory.
	*/
	virtual ~LESInterface() {
		delete m_statsFileStream;
	}

	/*! Initialize the linear equation solver, called from the
		framework before integration is started. */
	virtual void init(ModelInterface * model, IntegratorInterface * integrator,
		PrecondInterface * precond, JacobianInterface * jacobian) = 0;

	/*! In this function, the equation system is composed.
		This function is called from the integrator during iterations,
		at irregular intervals if modified Newton method is used.
		\param y The current prediction of the solution.
		\param ydot The corresponding result of f(y).
		\param residuals The current prediction of the solution.
		\param gamma The factor in front of the derivative df/dy (dt in case of Implicit Euler).
	*/
	virtual void setup(const double * y, const double * ydot, const double * residuals, double gamma) = 0;

	/*! In this function, the equation system is solved for a given right-hand-side.
		This function is called from the integrator in each iteration.
		\param rhs The right-hand-side of the equation system to solve for.
	*/
	virtual void solve(double * rhs) = 0;

	/*! Called from the framework to write create statistics file and write its header.
		\param logfilePath set a path to a statistics file.
		\param doRestart set this to true to append to an existing statistics file.
	*/
	virtual void writeStatisticsHeader(const IBK::Path & logfilePath, bool doRestart) { (void)logfilePath; (void)doRestart; }

	/*! Writes currently collected statistics.
		\param t Current simulation time point.
	*/
	virtual void writeStatistics(double t) { (void)t;}

	/*! Writes currently collected solver metrics/statistics to output.
		\param simtime Totel elapsed wall clock time of simulation in [s] (needed for percentage calculation)
		\param metricsFile If not nullptr, computer-readible metrics are written to the file.
	*/
	virtual void writeMetrics(double simtime, std::ostream * metricsFile=nullptr) { (void)simtime; (void)metricsFile; }

	/*! Updates cached statistics when using SUNDIALS solvers, which do not write
		statistics variables automatically.
	*/
	virtual void updateSUNDIALSStatistics() = 0;

	// Serialization/Deserialization

	/*! Computes and returns serialization size, by default returns  returns an invalid value (-1). */
	virtual std::size_t serializationSize() const {  return NON_SUPPORTED_FUNCTION; }

	/*! Stores content at memory location pointed to by dataPtr and increases
		pointer afterwards to point just behind the memory occupied by the copied data.
		Default implementation does nothing.
	*/
	virtual void serialize(void* & dataPtr) const { (void)dataPtr; }

	/*! Restores content from memory at location pointed to by dataPtr and increases
		pointer afterwards to point just behind the memory occupied by the copied data.
		Default implementation does nothing.
	*/
	virtual void deserialize(void* & dataPtr) { (void)dataPtr; }


	/*! Number of rhs evaluations for implicit Euler integrator. */
	unsigned int							m_statNumRhsEvals;

protected:

	/*! Pointer to the integrator interface: either CVODE or IDA or ImplicitEuler (not owned). */
	IntegratorInterface						*m_integrator;

	/*! Pointer to the underlying model (not owned). */
	ModelInterface							*m_model;

	/*! Common stream pointer to write statistics. */
	std::ostream							*m_statsFileStream;

	/*! Relative tolerance to compute epsilon in difference-quotient approximation. */
	double									m_relToleranceDQ;
	/*! Absolute tolerance to compute epsilon in difference-quotient approximation. */
	double									m_absToleranceDQ;


private:
	/*! Prevent copy constructor. */
	LESInterface(const LESInterface&);
	/*! Prevent copying. */
	void operator=(const LESInterface&);
};

} // namespace SOLFRA

#endif // SOLFRA_LESInterfaceH

