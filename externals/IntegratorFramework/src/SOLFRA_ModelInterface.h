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

#ifndef ModelInterfaceH
#define ModelInterfaceH

#include "SOLFRA_Constants.h"

#include <iosfwd>
#include <string>

#include <IBK_Time.h>

#include "SOLFRA_IntegratorInterface.h"

namespace SOLFRA {

class LESInterface;
//class IntegratorInterface;
class PrecondInterface;
class JacobianInterface;
class OutputScheduler;

/*!	\brief Declaration for class ModelInterface

	The class ModelInterface defines the interface functions for any model implementation that can be used
	within the integrator framework. If a model interface is implemented in a derived class, the framework
	with the available integrators and core integration loop can be used.
	\sa SolverControlFramework
*/
class ModelInterface {
public:
	/*! Different result types of the computation functions. */
	enum CalculationResult {
		/*! Operation successful. */
		CalculationSuccess,
		/*! Operation/calculation failed, but may be successful if re-attempted with different starting values
			or if a shorter time step were used. */
		CalculationRecoverableError,
		/*! Critical error encountered, program should abort. */
		CalculationAbort
	};

	/*! Default constructor. */
	ModelInterface() {}

	/*! Default destructor. */
	virtual ~ModelInterface() {}

	/*! Number of unknowns/states, size n. */
	virtual unsigned int n() const = 0;

	/*! Initial condition vector, size n.
		\return Returns a pointer to a linear memory array of size n() with the
				initial states.

		\warning The lifetime of the pointer may be limited. Specifically,
				 the pointer may be invalidated in a call to readRestartInfo().
	*/
	virtual const double * y0() const = 0;
	/*! Start time point in [s]. */
	virtual double t0() const = 0;
	/*! Initial time step. */
	virtual double dt0() const = 0;
	/*! End time point in [s]. */
	virtual double tEnd() const = 0;

	/*! Update state of model to new time point. */
	virtual CalculationResult setTime(double t) = 0;

	/*! Update state of model to new set of unknowns.
		This function must be implemented in derived classes.
	*/
	virtual CalculationResult setY(const double * y) = 0;

	/*! Stores the computed derivatives of the solution variables in the vector ydot.
		This function must be implemented by models who support an ODE type interface,
		which means they implement setY() and ydot(), but not setYdot() and residuals().
		\param ydot Pointer to linear memory array of size n() holding the time derivatives for each solution variable.
	*/
	virtual CalculationResult ydot(double * ydot) = 0;

	/*! Writes outputs at time t_out using the states y_out.
		This function is called for ODE-type models.
		The pointer y_out has limited lifetime and should only be used in this
		function and never be kept for later calls.
		This function may alter the state of the model implementation object to match
		the output time and solution.
		Default implementation does nothing.
		\param t_out Output time point (in seconds, simulation time).
		\param y_out Pointer to linear memory array of size n() containing the corresponding solution.
	*/
	virtual void writeOutputs(double t_out, const double * y_out) {
		(void)t_out; (void)y_out;
	}

	/*! Called at end of simulation for final output file caching/closing of output files. */
	virtual void writeFinalOutputs() {}

	/*! Function to convert relative simulation time t into a date/time string representation.
		Default implementation assumes simulation starts at midnight 2000 and returns a corresponding date/time string.
	*/
	virtual std::string simTime2DateTimeString(double t) const { return IBK::Time(0, t).toShortDateFormat(); }

	/*! Informs the model that a step was successfully completed.
		The time point and value vector passed to the function correspond to
		the current state in the integrator object.
		Default implementation does nothing.
		\note The stepCompleted() function is called first with t = t0() before the integration begins.
			  This can be used to do initialization of the model's discrete state changing code.
		\param t Time point reached at end of step.
		\param y Pointer to linear memory array of size n() containing the solution at this time point.
	*/
	virtual void stepCompleted(double t, const double * y) { (void)t; (void)y; }

	/*! Returns linear equation system solver.
		Default implementation returns nullptr, which means that the
		SundialsDense is used for both CVODE and IDA.
	*/
	virtual LESInterface * lesInterface() { return nullptr; }

	/*! Returns integrator to be used for this model.
		Default implementation returns nullptr, which results in CVODE integrator for
		ODE type models and IDA for DAE-type models.
	*/
	virtual IntegratorInterface * integratorInterface() { return nullptr; }

	/*! Returns model-specific pre-conditioner for use with iterative LES solvers.
		Default implementation returns nullptr (no preconditioner).
	*/
	virtual PrecondInterface * preconditionerInterface() { return nullptr; }

	/*! Returns model-specific Jacobian matrix generation method for use with iterative LES solvers.
		Default implementation returns nullptr (no jacobian generator).
	*/
	virtual JacobianInterface * jacobianInterface() { return nullptr; }

	/*! Returns output scheduler to be used with the framework.
		Default implementation returns nullptr (standard output scheduler).
	*/
	virtual OutputScheduler * outputScheduler() { return nullptr; }

	/*! Returns error weights to the integrator if defined. Otherwise nullptr.
		\todo make this function const
	*/
	virtual CalculationResult calculateErrorWeights(const double *y, double *weights) {(void) y; (void) weights; return CalculationAbort; }

	/*! Informs the integrator whether the model owns an error weighting function. */
	virtual bool hasErrorWeightsFunction() {return false; }

	/*! Computes and returns serialization size, by default returns SOLFRA_NOT_SUPPORTED_FUNCTION which means feature not supported. */
	virtual std::size_t serializationSize() const { return SOLFRA_NOT_SUPPORTED_FUNCTION; }

	/*! Stores model content at memory location pointed to by dataPtr and increases
		pointer afterwards to point just behind the memory occupied by the copied data.
		Default implementation does nothing.
	*/
	virtual void serialize(void* & dataPtr) const { (void)dataPtr; }

	/*! Restores model content from memory at location pointed to by dataPtr and increases
		pointer afterwards to point just behind the memory occupied by the copied data.
		Default implementation does nothing.
	*/
	virtual void deserialize(void* & dataPtr) { (void)dataPtr; }

	/*! Writes currently collected metrics/statistics of the model evaluation function to output.
		The collected statistics will be printed just below the LES solver statistics block.
		When re-implementing this function, please use the same formatting/layout for the table to
		give a consistent view.
		\param simtime Totel elapsed wall clock time of simulation in [s] (needed for percentage calculation)
		\param metricsFile If not nullptr, computer-readible metrics are written to the file.
	*/
	virtual void writeMetrics(double simtime, std::ostream * metricsFile = nullptr) { (void)simtime; (void)metricsFile; }

private:
	/*! Copy constructor, private and not implemented : model may not be copied. */
	ModelInterface(const ModelInterface &);
	/*! Assignment operator, private and not implemented : model may not be copied. */
	const ModelInterface & operator=(const ModelInterface &);
};

} // namespace SOLFRA

#endif // ModelInterfaceH
