#ifndef SOLFRA_JacobianInterfaceH
#define SOLFRA_JacobianInterfaceH

#include <cstdlib>

#include "SOLFRA_Constants.h"

namespace SOLFRA {

class ModelInterface;

/*! This class defines the interface functions for generating a Jacobian matrix
	and computing a matrix/vector multiplication.

	The Jacobian-Matrix is defined as df/dy in case of ODE-type models.
*/
class JacobianInterface {
public:

	/*! Constructor. */
	JacobianInterface() :
		m_linearProblem(false),
		m_linearProblemSetupDone(false)
	{
	}

	/*! Virtual destructor.
		Classes re-implementing the JacobianInterface should use
		the destructor to release allocated memory.
	*/
	virtual ~JacobianInterface() {}

	/*! Initialize the Jacobian matrix generator, called from the
		framework before integration is started. */
	virtual void init(ModelInterface * model) = 0;

	/*! In this function the Jacobian matrix is composed.
		This function is called from the linear equation solver during iterations.
		\param t The current time point.
		\param y The current prediction of the solution.
		\param ydot The corresponding result of f(y).
		\param residuals The current prediction of the solution.
		\param gamma The factor in front of the derivative df/dy (time step in case of implicit Euler).
	*/
	virtual int setup(double t, const double * y, const double * ydot, const double * residuals,
		double gamma) = 0;

	/*! Function implementing J*v operation. */
	virtual int jacTimesVec(const double * v, double * Jv) const = 0;

	/*! Holds number of RHS function evaluations (ydot()/residual() calls) used for generating
		the Jacobian matrix.
	*/
	virtual unsigned int nRHSEvals() const = 0;

	// Serialization/Deserialization

	/*! Computes and returns serialization size, by default returns 0 which means feature not supported. */
	virtual std::size_t serializationSize() const { return NON_SUPPORTED_FUNCTION; }

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


	/*! If true, the setup can be made only once since the problem is linear and df/dy is constant.
		This flag is not set within the integrator framework. Typically, the model shall set this flag
		when the computed problem is linear (which only the model can know).
	*/
	bool	m_linearProblem;
	/*! If true, the setup was already done for this linear problem, and the Jacobian matrix
		won't be updated in the next call to setup.
	*/
	bool	m_linearProblemSetupDone;

};

} // namespace SOLFRA

#endif // SOLFRA_JacobianInterfaceH

