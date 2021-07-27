#ifndef SOLFRA_LESBTridiagH
#define SOLFRA_LESBTridiagH

#include "SOLFRA_LESInterfaceDirect.h"
#include "SOLFRA_IntegratorInterface.h"

#include <vector>
#include <iosfwd>

namespace SOLFRA {

class ModelInterface;

/*!  A wrapper class for Sundials' own BTridiag solver.
	 The constructor simply takes the data that the BTridiag solver
	 of sundials requires. The init() function the registers
	 the band solver with the sundials solver object exposed
	 by the integrator class.
*/
class LESBTridiag : public LESInterfaceDirect {
public:

	LESBTridiag(unsigned int nblocks, unsigned int blocksize);

	~LESBTridiag();

	/*! Initialize the linear equation solver, called from the
		framework before integration is started. */
	virtual void init(ModelInterface * model, IntegratorInterface * integrator,
					  PrecondInterface * precond, JacobianInterface * jacobian) override;

	/*! Re-implemented from LESInterface::setup(). 	*/
	virtual void setup(const double * y, const double * ydot, const double * residuals, double gamma) override;

	/*! Re-implemented from LESInterface::solve(). */
	virtual void solve(double * rhs) override;

private:
	/*! Size of the equation system. */
	unsigned int							m_n;
	unsigned int							m_nblocks;
	unsigned int							m_blocksize;

	/*! Pointer to the underlying model. */
	ModelInterface							*m_modelODE;

	/*! Pointer to jacobian matrix (Sundials:DlsMat Format) */
	void									*m_jacobian;
	/*! Used to store individual y elements. */
	std::vector<double>						m_yMod;
	/*! Used to store individual ydot elements. */
	std::vector<double>						m_ydotMod;
	/*! Used to store individual right hand side of ODE models. */
	std::vector<double>						m_FMod;
	/*! Used to store differences added to the individual y elements. */
	std::vector<double>						m_ydiff;
	/*! Stores the modified residuals/ydot values during the FD algorithm. */
	std::vector<double>						m_residualsMod;
	/*! ID nummbers for ODE equations (size n). */
	std::vector<double>						m_odeIDs;
	std::vector<long int>					m_pivots;
};

} // namespace SOLFRA


#endif // SOLFRA_SundialsBTridiagLESH
