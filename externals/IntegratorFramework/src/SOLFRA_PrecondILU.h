#ifndef SOLFRA_PrecondILUH
#define SOLFRA_PrecondILUH

#include "SOLFRA_PrecondInterface.h"

#include <vector>

namespace IBKMK {
	class SparseMatrix;
}

namespace SOLFRA {

class IntegratorInterface;
class JacobianSparse;
class ModelInterface;

/*! An ILU pre-conditioner supporting the SUNDIALS precond interface.
	The ILU algorithm is currently implemented for sparse matrix structures in format EID and CSR.

	During setup, the data of the provided Jacobian matrix df/dy is copied into the local jacobian sparse
	memory array and then done in-place.

	PrecondILU only operates on the general SparseMatrix interface and does not know of the details of the
	unterlying SparseMatrix structure.
*/
class PrecondILU : public PrecondInterface {
public:
	/*! Initializes PrecondILU. */
	explicit PrecondILU(PreconditionerType precondType = PrecondInterface::Right );

	/*! Destructor, releases band matrix memory. */
	~PrecondILU() override;

	/*! Returns type of precondition (where it should be applied in context of the iteration linear equation solver). */
	virtual PreconditionerType preconditionerType() const override { return m_precondType; }

	/*! Initialize the preconditioner, called from the
		framework before integration is started. */
	virtual void init(ModelInterface * model, IntegratorInterface * integrator,
					  const JacobianInterface * jacobianInterface) override;

	/*! In this function, the preconditioner matrix is composed an LU-factorised.
		This function is called from the linear equation solver during iterations.
		\param y The current prediction of the solution.
		\param residuals The currentl prediction of the solution.
	*/
	virtual int setup(double t, const double * y, const double * ydot, const double * residuals,
		bool jacOk, bool & jacUpdated, double gamma) override;

	virtual int solve(double t, const double * y, const double * ydot, const double * residuals,
		const double * r, double * z, double gamma, double delta, int lr) override;

	/*! Holds number of RHS function evaluations (ydot()/residual() calls) used for generating
		the preconditioner.
	*/
	virtual unsigned int nRHSEvals() const override { return 0; }

	/*! Computes and returns serialization size. */
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

private:
	/*! Jacobian matrix, if provided, the Jacobian data is transferred within
		setup() from the Jacobian data, rather than being computed anew via DQ approximation. */
	const JacobianSparse					*m_jacobianSparse;

	/*! Sparse matrix implementation (owned, because we use an in-place ILU operation and do not want
		to destroy the original Jacobian memory which is needed for the iterative solver).
	*/
	IBKMK::SparseMatrix						*m_precondMatrix;

};

} // namespace SOLFRA

#endif // SOLFRA_PrecondILU
