#ifndef SOLFRA_LESBandH
#define SOLFRA_LESBandH

#include "SOLFRA_LESInterfaceDirect.h"
#include "SOLFRA_IntegratorInterface.h"

#include <vector>
#include <iosfwd>

namespace IBKMK {
	class BandMatrix;
}

namespace SOLFRA {

class IntegratorSundialsCVODE;
class IntegratorSundialsIDA;

/*! Implementation of a band matrix solver. */
class LESBand : public LESInterfaceDirect {
public:
	/*! Band matrix solver constructor, takes upper and lower bandwidth mu and ml. */
	LESBand(unsigned mu, unsigned int ml);

	/*! Destructor, releases band matrix memory. */
	~LESBand();

	/*! Re-implemented from LESInterface::init(). 	*/
	void init(ModelInterface * model, IntegratorInterface * integrator,
			  PrecondInterface * precond, JacobianInterface * jacobian) override;

	/*! Re-implemented from LESInterface::setup(). 	*/
	virtual void setup(const double * y, const double * ydot, const double * residuals, double gamma) override;

	/*! Re-implemented from LESInterface::solve(). */
	virtual void solve(double * rhs) override;

	/*! Computes and returns serialization size, by default returns returns an invalid value (-1). */
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
	/*! Upper bandwidth. */
	unsigned int							m_mu;
	/*! Lower bandwidth. */
	unsigned int							m_ml;

	/*! Band matrix implementation (owned). */
	IBKMK::BandMatrix						*m_jacobian;
	/*! Used to store modified y elements. */
	std::vector<double>						m_yMod;
	/*! Used to store individual ydot elements. */
	std::vector<double>						m_ydotMod;
	/*! Used to store individual right hand side of ODE models. */
	std::vector<double>						m_FMod;
	/*! Used to store differences added to the individual y elements. */
	std::vector<double>						m_ydiff;
};

} // namespace SOLFRA


#endif // SOLFRA_LESBandH
