#ifndef SOLFRA_LESBiCGStabP1H
#define SOLFRA_LESBiCGStabP1H

#include <vector>

#include "SOLFRA_LESInterfaceIterative.h"
#include "SOLFRA_PrecondInterface.h"

namespace SOLFRA {

class IntegratorSundialsCVODE;
class IntegratorSundialsIDA;

/*!  A wrapper class for Sundials' own gmres solver.
	 The constructor simply takes the data that the GMRES solver
	 of sundials requires. The init() function the registers
	 the gmres solver with the sundials solver object exposed
	 by the integrator class.
*/
class LESBiCGStabP1 : public LESInterfaceIterative {
public:
	LESBiCGStabP1();

	/*! Initialize the linear equation solver, called from the
		framework before integration is started. */
	virtual void init(ModelInterface * model, IntegratorInterface * integrator,
					  PrecondInterface * precond, JacobianInterface * jacobian) override;

	virtual void setup(const double * y, const double * ydot, const double * residuals, double gamma) override;

	virtual void solve(double * rhs) override;

private:

	/*! Error limit for solving linear equation system. */
	double									m_deltaNorm;

	/* Serial BiCGTestCode, implemented for extension to mpi test. */

	/* all latins are vectors. */
	/*! reuse vector for A^Tr_0 */
	std::vector<double>						m_f;

	/*! residual of bicgstab */
	std::vector<double>						m_r;

	/*! residual of bicgstab */
	std::vector<double>						m_r0;

	/*!  */
	std::vector<double>						m_s;

	/*!  */
	std::vector<double>						m_t;

	/*!  */
	std::vector<double>						m_q;

	/*!  */
	std::vector<double>						m_u;

	/*!  */
	std::vector<double>						m_v;

	/*!  */
	std::vector<double>						m_x;

	/*! Since this algorithm eliminated m_p,
		we require m_z = alpha_n * m_p = alpha_n * r_1 + beta * m_z - alpha * beta * omega1 * m_v1
		to update x_n.
	*/
	std::vector<double>						m_z;

	/*! Used to store individual weighting function for scaling the residuals. */
	std::vector<double>						m_weights;


};

} // namespace SOLFRA


#endif // SOLFRA_LESBiCGStabP1H
