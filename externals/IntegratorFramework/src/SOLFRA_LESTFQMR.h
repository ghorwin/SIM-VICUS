#ifndef SOLFRA_LESTFQMRH
#define SOLFRA_LESTFQMRH

#include "SOLFRA_LESInterfaceIterative.h"

namespace SOLFRA {

/*!  A wrapper class for Sundials' own TFQMR solver.
	 The constructor simply takes the data that the TFQMR solver
	 of sundials requires. The init() function the registers
	 the TFQMR solver with the sundials solver object exposed
	 by the integrator class.
*/
class LESTFQMR : public LESInterfaceIterative {
public:
	/*! Standard constructor. */
	LESTFQMR();

	/*! Destructor. */
	~LESTFQMR() override;

	/*! Initializes the linear equation solver, called from the
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

	/*! Called from the framework to write create statistics file and write its header. */
	virtual void writeStatisticsHeader(const IBK::Path & logfilePath, bool doRestart) override;

	/*! Writes currently collected statistics. */
	virtual void writeStatistics(double t) override;

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

};

} // namespace SOLFRA


#endif // SOLFRA_SundialsBandLESH
