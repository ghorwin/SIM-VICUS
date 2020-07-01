#ifndef SOLFRA_LESInterfaceDirectH
#define SOLFRA_LESInterfaceDirectH

#include "SOLFRA_LESInterface.h"

namespace SOLFRA {

/*! This class defines the interface functions for a linear equation system solver.

	Special version for native sundials solver:
	- prefixed Sundials
	- re-implemented class type SundialsLinearEquationSolver provides type for
	  each native solver and provides required parameters,
	- functionality defined in LinearEquationSolverInterface is not implemented
	  in these cases

	Regular LES Implementations:
	- implement init() functions
	- implement setup() and solve(), may use model interface to let model compute
	  values, for example DQ

	  Provided default implementations:
	  Direct solvers:
	  - LESDense
	  - LESBand
	  - LESBTridiag
*/
class LESInterfaceDirect : public LESInterface {
public:

	/*! Default constructor */
	LESInterfaceDirect() :
		m_statNumJacEvals(0)
	{}

	/*! Called from the framework to write create statistics file and write its header.
		\param logfilePath set a path to a statistics file.
		\param doRestart set this to true to append to an existing statistics file.
	*/
	virtual void writeStatisticsHeader(const std::string & logfilePath, bool doRestart) override;

	/*! Writes currently collected statistics.
		\param t Current simulation time point.
	*/
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

	/*! Number of Jacobian evaluations (df/dy). */
	unsigned int							m_statNumJacEvals;

};

} // namespace SOLFRA




#endif // SOLFRA_LESInterfaceDirectH
