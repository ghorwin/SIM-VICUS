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

#ifndef IntegratorInterfaceH
#define IntegratorInterfaceH

#include "SOLFRA_Constants.h"

#include <iosfwd>

#include <IBK_Path.h>

namespace SOLFRA {

class ModelInterface;
class LESInterface;
class PrecondInterface;
class JacobianInterface;

/*!	\brief Declaration for class IntegratorInterface.

	This abstract interface defines the necessary functionality of an ODE integrator.
	The integrators provided by the framework implement this interface. User-defined integrators
	may also implement this interface and can then be used within the SolverControlFramework.
	\sa SolverControlFramework
*/
class IntegratorInterface {
public:
	/*! Return value types for step(). */
	enum StepResultType {
		StepSuccess			= 1,
		StepCriticalError	= -1,
		StepAttemptRestart	= -2
	};

	/*! Default constructor. */
	IntegratorInterface() : m_numThreads(1) {}

	/*! Default destructor. */
	virtual ~IntegratorInterface() {}

	/*! Returns a unique identifier/display name for this integrator. */
	virtual const char * identifier() const = 0;

	/*! Initializes the Integrator.
		\param model The physical model instance.
		\param t0 Starting time point.
		\param y0 Pointer to linear memory array of size model->n() holding the initial conditions.
		\param lesSolver Linear equation system solver, can be nullptr if (explicit) solver doesn't need LES solver functionality.
		\param precond Preconditioner, in case that an iterative linear equation system solver is used.
			If nullptr, no preconditioner is specified.
		\param jacobain Jacobian matrix implementation (generation and J*v), in case that an iterative linear
			equation system solver is used. If nullptr, default Jacobian functionality via direction derivatives is specified.
	*/
	virtual void init(ModelInterface * model, double t0, const double * y0,
					  LESInterface * lesSolver,
					  PrecondInterface * precond,
					  JacobianInterface * jacobian) = 0;

	/*! Advances the solution from the current time to the next, thereby
		adjusting the time step.
		If the step was completed sucessfully, the functions t() and dt() can be
		used to query the new time point and the step size just used, respectively.
		\return Returns an enum value indicating the outcome of the step. For
				negative values, the state of the solver is undetermined.
	*/
	virtual StepResultType step() = 0;

	/*! Returns the current time point in simulation time in [s].
		Current time point is updated in a call to step().
	*/
	virtual double t() const = 0;

	/*! Returns the time step in [s] that was used in the last step.
		Time step size is updated in a call to step().
	*/
	virtual double dt() const = 0;

	/*! Returns a pointer to the memory array with interpolated states at the
		given output time.
		\param t_out A time point between the current time point and the
					 previous time point.
		\return A constant pointer to the memory array holding the interpolated data.
		\warning The lifetime of the pointer returned from this function may be limited.
				 At latest after a call to step() or another y_out() call, the pointer
				 is invalidated and has to be queried again.
	*/
	virtual const double * yOut(double t_out) const = 0;

	/*! Called from the framework to write create statistics file and write its header. */
	virtual void writeStatisticsHeader(const IBK::Path & /* logfilePath */, bool /* doRestart */) {}

	/*! Writes currently collected statistics to log file. */
	virtual void writeStatistics() {}

	/*! Writes currently collected solver metrics/statistics to output.
		\param simtime Totel elapsed wall clock time of simulation in [s] (needed for percentage calculation)
		\param metricsFile If not nullptr, computer-readible metrics are written to the file.
	*/
	virtual void writeMetrics(double simtime, std::ostream * metricsFile=nullptr) { (void)simtime; (void)metricsFile; }

	// Serialization/Deserialization

	/*! Computes and returns serialization size, by default returns SOLFRA_NOT_SUPPORTED_FUNCTION which means feature not supported. */
	virtual std::size_t serializationSize() const { return SOLFRA_NOT_SUPPORTED_FUNCTION; }

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

	/*! Number of threads that shoud be used in parallel openmp vector implementations. */
	int						m_numThreads;

private:
	/*! Copy constructor is private and not implemented - copying not allowed. */
	IntegratorInterface(const IntegratorInterface &);
};

} // namespace SOLFRA

#endif // IntegratorInterfaceH
