#ifndef SOLFRA_LESGMRESH
#define SOLFRA_LESGMRESH

#include "SOLFRA_LESInterfaceIterative.h"

#include <vector>
#include <iosfwd>

namespace SOLFRA {


class JacobianInterface;
class PrecondInterface;
class ModelInterface;
class IntegratorInterface;

/*!  \todo Anne: fix memory leaks and update documentation
*/
class LESGMRES : public LESInterfaceIterative {
public:
	/*! Initialize the linear equation solver, called from the
		framework before integration is started.
	*/
	virtual void init(ModelInterface * model, IntegratorInterface * integrator, PrecondInterface * precond,
					  JacobianInterface * jacobian) override;

	/*! Setup of equation system.
		\sa LESInterface::setup
	*/
	virtual void setup(const double * y, const double * ydot, const double * residuals, double gamma) override;

	/*! Solves equation system.
		\sa LESInterface::solve
	*/
	virtual void solve(double * rhs) override;

	/*! Computes and returns serialization size, by default returns 0 which means feature not supported. */
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

};

} // namespace SOLFRA


#endif // SOLFRA_SundialsBandLESH
