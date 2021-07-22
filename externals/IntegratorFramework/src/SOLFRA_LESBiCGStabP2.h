#ifndef SOLFRA_LESBiCGStabP2H
#define SOLFRA_LESBiCGStabP2H

#include <vector>

#include "SOLFRA_LESInterfaceIterative.h"
#include "SOLFRA_PrecondInterface.h"

namespace SOLFRA {

class IntegratorSundialsCVODE;
class IntegratorSundialsIDA;

/*! Implements an BiCGStab method which uses convergence loop unrooling.
 *
 * This code is crucial allows to split the memory region in elements dimension
 * and is thus better usable with MPI implementation then whith OPENMP.
 * The initial idea is a
*/
class LESBiCGStabP2 : public LESInterfaceIterative {
public:
	LESBiCGStabP2();

	/*! Initialize the linear equation solver, called from the
		framework before integration is started. */
	virtual void init(ModelInterface * model, IntegratorInterface * integrator,
					  PrecondInterface * precond, JacobianInterface * jacobian) override;

	virtual void setup(const double * y, const double * ydot, const double * residuals, double gamma) override;

	virtual void solve(double * rhs) override;

private:

	/* Serial BiCGTestCode, implemented for extension to mpi test. */

	/* all latins are vectors. */
	/*!  */
	std::vector<double>						m_rRoof;

	/*! residual of bicgstab */
	std::vector<double>						m_r;

	/*! residual of bicgstab */
	std::vector<double>						m_r0;

	/*!  */
	std::vector<double>						m_p;

	/*!  */
	std::vector<double>						m_v;

	/*!  */
	std::vector<double>						m_w;

	/*!  */
	std::vector<double>						m_wRoof;

	/*!  */
	std::vector<double>						m_s;

	/*!  */
	std::vector<double>						m_t;

	/*!  */
	std::vector<double>						m_u;

	/*!  */
	std::vector<double>						m_X;

};

} // namespace SOLFRA


#endif // SOLFRA_LESBiCGStabP2H
