#ifndef SOLFRA_LESDenseH
#define SOLFRA_LESDenseH

#include "SOLFRA_LESInterfaceDirect.h"
#include "SOLFRA_IntegratorInterface.h"

#include <vector>

namespace IBKMK {
	class DenseMatrix;
}

namespace SOLFRA {

class IntegratorSundialsCVODE;
class IntegratorImplicitEuler;

/*!  A wrapper class for Sundials' own Dense solver.
	 The constructor simply takes the data that the Dense solver
	 of sundials requires. The init() function the registers
	 the Dense solver with the sundials solver object exposed
	 by the integrator class.
*/
class LESDense : public LESInterfaceDirect {
public:
	/*! Default constructor. */
	LESDense();

	/*! Destructor. */
	virtual ~LESDense();

	/*! Initialize the linear equation solver, called from the
		framework before integration is started. */
	virtual void init(ModelInterface * model, IntegratorInterface * integrator,
					  PrecondInterface * precond, JacobianInterface * jacobian) override;

	virtual void setup(const double * y, const double * ydot, const double * residuals, double gamma) override;

	/*! In this function, the equation system is solved for a given right-hand-side.
		This function is called from the integrator in each iteration.
		\param rhs The right-hand-side of the equation system to solve for.
	*/
	virtual void solve(double * rhs) override;

	/*! Computes and returns serialization size, by default returns  returns an invalid value (-1). */
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

	/*! Size of the equation system. */
	unsigned int							m_n;

	/*! Used to store differences added to the individual y elements. */
	IBKMK::DenseMatrix						*m_jacobian;

	/*! Used to store individual y elements. */
	std::vector<double>						m_yMod;
	/*! Used to store individual ydot elements. */
	std::vector<double>						m_ydotMod;
	/*! Used to store differences added to the individual y elements. */
	std::vector<double>						m_ydiff;
	/*! Stores the modified residuals/ydot values during the FD algorithm. */
	std::vector<double>						m_residualsMod;
	/*! ID nummbers for ODE equations (size n). */
	std::vector<double>						m_odeIDs;


};

} // namespace SOLFRA


#endif // SOLFRA_SundialsDenseLESH
