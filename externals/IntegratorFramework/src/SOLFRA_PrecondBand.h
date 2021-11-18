#ifndef SOLFRA_PrecondBandH
#define SOLFRA_PrecondBandH

#include <iosfwd>

#include "SOLFRA_PrecondInterface.h"

#include <vector>
#include <iosfwd>

namespace IBKMK {
	class BandMatrix;
}

namespace SOLFRA {

class ModelInterface;
class IntegratorInterface;
class JacobianInterface;

/*! A band pre-conditioner for use with Sundials solvers. */
class PrecondBand : public PrecondInterface {
public:
	/*! Constructor, takes lower and upper half bandwidths as arguments.
		\param ml Lower half-bandwidth.
		\param mu Upper half-bandwidth.
		\param precondType The preconditioner type as queried by preconditionerType().
	*/
	PrecondBand(unsigned int ml, unsigned int mu, PreconditionerType precondType = PrecondInterface::Right );

	/*! Destructor. */
	~PrecondBand() override;

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

	/*! Applies preconditioner to vector r, and returns resultant vector z).*/
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
	/*! Cached system size. */
	unsigned int							m_n;

	/*! Lower half-bandwidth. */
	unsigned int							m_ml;

	/*! Upper half-bandwidth. */
	unsigned int							m_mu;

	/*! Jacobian matrix, if provided, the Jacobian data is transferred within
		setup() from the Jacobian data, rather than being computed anew via DQ approximation. */
	const JacobianInterface					*m_jacobianInterface;

	/*! Jacobian matrix (in LU factorization). */
	IBKMK::BandMatrix						*m_jacobian;
	/*! Jacobian matrix (not LU factorized) with respect to df/dy (part of J = I - gamma * df/dy). */
	IBKMK::BandMatrix						*m_partialJacobianCopy;
};

} // namespace SOLFRA

#endif // SOLFRA_PrecondBand
