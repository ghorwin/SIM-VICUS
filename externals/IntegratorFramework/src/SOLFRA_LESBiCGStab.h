#ifndef SOLFRA_LESBiCGStabH
#define SOLFRA_LESBiCGStabH

#include "SOLFRA_LESInterfaceIterative.h"

namespace SOLFRA {

class IntegratorSundialsCVODE;

/*!  A wrapper class for Sundials' own BiCGStab solver.
	 The constructor simply takes the data that the BiCGStab solver
	 of sundials requires. The init() function the registers
	 the BiCGStab solver with the sundials solver object exposed
	 by the integrator class.
*/
class LESBiCGStab : public LESInterfaceIterative {
public:
	/*! Initialize the linear equation solver, called from the
		framework before integration is started. */
	virtual void init(ModelInterface * model, IntegratorInterface * integrator,
					  PrecondInterface * precond, JacobianInterface * jacobian) override;

	/*! Setup of equation system.
		\sa LESInterface::setup
	*/
	virtual void setup(const double * y, const double * ydot, const double * residuals, double gamma) override;

	/*! Solves equation system.
		\sa LESInterface::solve
	*/
	virtual void solve(double * rhs) override;

	// TODO : serialization routines
};

} // namespace SOLFRA


#endif // SOLFRA_SundialsBandLESH
