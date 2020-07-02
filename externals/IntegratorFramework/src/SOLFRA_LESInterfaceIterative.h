#ifndef SOLFRA_LESInterfaceIterativeH
#define SOLFRA_LESInterfaceIterativeH

#include "SOLFRA_LESInterface.h"

namespace SOLFRA {

/*! This class defines the interface functions for a iterative linear equation system solvers.

	Provided default implementations:
	Iterative solvers:
	- LESGMRES
	- LESBiCGStab
*/
class LESInterfaceIterative : public LESInterface {
public:

	/*! Constructor initialises statistics counters. */
	LESInterfaceIterative() :
		m_linIterConvCoeff(0),
		m_maxKrylovDim(50),
		m_statNumLinIters(0),
		m_statNumLinConvFails(0),
		m_statNumPrecEvals(0),
		m_statNumPrecSolves(0),
		m_statNumJacTimesEvals(0),
		m_precond(NULL),
		m_jacobian(NULL)
	{}

	/*! Called from the framework to create statistics file and write its header. */
	virtual void writeStatisticsHeader(const IBK::Path & logfilePath, bool doRestart) override;

	/*! Write statistics for iterative les solvers. */
	virtual void writeStatistics(double t) override;

	/*! Updates cached statistics when using SUNDIALS solvers, which do not write
		statistics variables automatically.
	*/
	virtual void updateSUNDIALSStatistics() override;

	/*! Writes currently collected solver metrics/statistics to output.
		\param simtime Totel elapsed wall clock time of simulation in [s] (needed for percentage calculation)
		\param metricsFile If not NULL, computer-readible metrics are written to the file.
	*/
	virtual void writeMetrics(double simtime, std::ostream * metricsFile=NULL) override;


	/***********************************************
	 *
	 * common configuration values and flags
	 *
	 ************************************************/


	/*! Safety factor for residual control in linear iteration. */
	double			m_linIterConvCoeff;

	/*! Maximum Krylov subspace dimension. */
	unsigned int	m_maxKrylovDim;


	/***********************************************
	 *
	 * Statistics.
	 *
	 ************************************************/

	/*! Number of linear iterations for implicit Euler integrator. */
	unsigned int							m_statNumLinIters;
	/*! Number of linear convergence fails for implicit Euler integrator. */
	unsigned int							m_statNumLinConvFails;
	/*! Number of preconditioner evaluations for implicit Euler integrator. */
	unsigned int							m_statNumPrecEvals;
	/*! Number of preconditioner solution steps for implicit Euler integrator. */
	unsigned int							m_statNumPrecSolves;
	/*! Number of J*v evaluations for implicit Euler integrator. */
	unsigned int							m_statNumJacTimesEvals;

protected:

	/*! Pointer to preconditioner interface (not owned). */
	PrecondInterface						*m_precond;

	/*! Jacobian matrix implementation (not owned). */
	JacobianInterface						*m_jacobian;


};

} // namespace SOLFRA


#endif // SOLFRA_LESInterfaceIterativeH
