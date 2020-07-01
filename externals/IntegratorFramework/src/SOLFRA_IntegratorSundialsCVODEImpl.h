#ifndef SOLFRA_IntegratorSundialsCVODEImplH
#define SOLFRA_IntegratorSundialsCVODEImplH

#include <iostream>

#include <sundials/sundials_nvector.h>

using namespace std;


namespace SOLFRA {

class ModelInterface;
class PrecondInterface;
class JacobianInterface;

/*! Implementation class for IntegratorSundialsCVODE. */
class IntegratorSundialsCVODEImpl {
public:
	/*! Default constructor. */
	IntegratorSundialsCVODEImpl();
	/*! For cleanup of CVODE specific memory. */
	~IntegratorSundialsCVODEImpl();

	/*! Pointer to the model used by this integrator. */
	ModelInterface			*m_model;

	/*! Pointer to preconditioner, if any (set in init()). */
	PrecondInterface		*m_precond;

	/*! Pointer to Jacobian matrix implementation, if any (set in init()). */
	JacobianInterface		*m_jacobian;

	/*! Cvode memory pointer. */
	void					*m_mem;
	/*! Vector for state variables, used in the CVODE solver.
		Do not retrieve state variables from this vector within f()!
		CVODE changes memory pointer frequently!
	*/
	N_Vector				m_yStorage;

	/*! Vector used to store interpolated output values after a successful call to CVODE. */
	N_Vector				m_yStorageOutput;
	/*! Backup vector for the m_yStorage vector, so that the solver can repeat a step. */
	N_Vector				m_yStorageLast;
	/*! Vector for local error estimates. */
	N_Vector				m_errEstimates;
	/*! Vector for absolute tolerances (different, depending on balance equations). */
	N_Vector				m_absTolVec;
	/*! Vector for error weights. */
	N_Vector				m_errWeights;
	/*! Cached time point in [s] (simtime) after last step. */
	double					m_t;
	/*! Cached time step in [s] after last step. */
	double					m_dt;

	// statistics variables
	std::ostream			*m_statsFileStream;

	long int				m_statNumSteps;
	long int				m_statNumRHSEvals;
	long int				m_statNumLinSetups;
	long int				m_statNumNIters;
	long int				m_statNumNCFails;
	long int				m_statNumErrFails;
	int						m_statMethodOrder;	///< Method order use in last step.
	double					m_statTimeStepSize;	///< Time step size used in last step.
}; // class IntegratorSundialsCVODEImpl

} // namespace SOLFRA

#endif // SOLFRA_IntegratorSundialsCVODEImplH
