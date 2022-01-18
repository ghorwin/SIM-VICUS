#ifndef SOLFRA_PrecondInterfaceH
#define SOLFRA_PrecondInterfaceH

#include "SOLFRA_Constants.h"

#include <string>
#include <iostream> // needed for delete operator in destructor

#include <IBK_Path.h>

namespace SOLFRA {

class ModelInterface;
class IntegratorInterface;
class JacobianInterface;

/*! This class defines the interface functions for a left/right sided preconditioner.
*/
class PrecondInterface {
public:

	/*! Different types of preconditioner. */
	enum PreconditionerType {
		None,
		Left,
		Right,
		Both
	};

	/*! Default constructor */
	PrecondInterface(PreconditionerType precondType = PrecondInterface::Left):
		m_statsFileStream(nullptr),
		m_integrator(nullptr),
		m_model(nullptr),
		m_precondType(precondType)
	{
	}


	/*! Virtual destructor.
		Classes re-implementing the PrecondInterfaceInterface should use
		the destructor to release allocated memory.
	*/
	virtual ~PrecondInterface() {
		delete m_statsFileStream;
	}

	/*! Returns type of precondition (where it should be applied in context of the iteration linear equation solver). */
	virtual PreconditionerType preconditionerType() const = 0;

	/*! Initialize the preconditioner, called from the
		framework before integration is started.
		\param model Pointer to model implementation object.
		\param integrator Pointer to integrator object (only needed for specific Sundials features).
		\param jacobian Pointer to Jacobian implementation (if nullptr, no dedicated Jacobian implementation is
				provided and the preconditioner should implement its own Jacobian generation method).
	*/
	virtual void init(ModelInterface * model, IntegratorInterface * integrator, const JacobianInterface * jacobian) = 0;

	/*! In this function, the preconditioner matrix is composed an LU-factorised.
		This function is called from the linear equation solver during iterations.
		\param y The current prediction of the solution.
		\param residuals The current prediction of the solution.
	*/
	virtual int setup(	double t,
						const double * y,
						const double * ydot,
						const double * residuals,
						bool jacOk,
						bool & jacUpdated,
						double gamma) = 0;

	/*! Function implementing CVSpilsPrecSolveFn.

		The user-supplied preconditioner solve function PrecSolve
		is to solve a linear system P z = r in which the matrix P is
		one of the preconditioner matrices P1 or P2, depending on the
		type of preconditioning chosen.

		A function PrecSolve must have the prototype given below.
		Its parameters are as follows:

		\param t      is the current value of the independent variable.

		\param y      is the current value of the dependent variable vector.

		\param fy     is the vector f(t,y).

		\param r      is the right-hand side vector of the linear system.

		\param z      is the output vector computed by PrecSolve.

		\param gamma  is the scalar appearing in the Newton matrix.

		\param delta  is an input tolerance for use by PSolve if it uses
			   an iterative method in its solution.  In that case,
			   the residual vector Res = r - P z of the system
			   should be made less than delta in weighted L2 norm,
			   i.e., sqrt [ Sum (Res[i]*ewt[i])^2 ] < delta.
			   Note: the error weight vector ewt can be obtained
			   through a call to the routine CVodeGetErrWeights.

		\param lr     is an input flag indicating whether PrecSolve is to use
			   the left preconditioner P1 or right preconditioner
			   P2: lr = 1 means use P1, and lr = 2 means use P2.

		\param user_data  is a pointer to user data - the same as the user_data
				parameter passed to the CVodeSetUserData function.

		\param tmp    is a pointer to memory allocated for an N_Vector
			   which can be used by PSolve for work space.

		\return Returned value:
		The value to be returned by the PrecSolve function is a flag
		indicating whether it was successful.  This value should be
		  0 if successful,
		  positive for a recoverable error (step will be retried),
		  negative for an unrecoverable error (integration is halted).
		-----------------------------------------------------------------

		\sa cvode_spils.h
	*/
	virtual int solve(double t, const double * y, const double * ydot, const double * residuals,
		const double * r, double * z, double gamma, double delta, int lr) = 0;

	/*! Called from the framework to write create statistics file and write its header.
		\param logfilePath set a path to a statistics file.
		\param doRestart set this to true to append to an existing statistics file.
	*/
	virtual void writeStatisticsHeader(const IBK::Path & logfilePath, bool doRestart) { (void)logfilePath; (void)doRestart; }

	/*! Writes currently collected statistics.
		\param t Current simulation time point.
	*/
	virtual void writeStatistics(double t) { (void)t; }

	/*! Holds number of RHS function evaluations (ydot()/residual() calls) used for generating
		the preconditioner.
	*/
	virtual unsigned int nRHSEvals() const = 0;

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

protected:
	/*! Statistics file stream. */
	std::ostream							*m_statsFileStream;

	/*! Pointer to the underlying model (not owned) */
	IntegratorInterface						*m_integrator;

	/*! Pointer to the underlying model. */
	ModelInterface							*m_model;

	/*! The preconditioner type, may be: Left, Right */
	PreconditionerType						m_precondType;

};

} // namespace SOLFRA

#endif // SOLFRA_PrecondInterfaceH

