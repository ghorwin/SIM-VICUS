#ifndef SOLFRA_IntegratorErrorControlledH
#define SOLFRA_IntegratorErrorControlledH

#include "SOLFRA_IntegratorInterface.h"

#include <vector>

namespace SOLFRA {


/*! Provides common data structures for error control.
	Provides default implementations for Integrators:
	\dot Implicit Euler
	\dot CVODE
	\dot Runge Kutta
*/
class IntegratorErrorControlled : public IntegratorInterface {
public:

	/*! Constructor, initializes relTol and absTol vectors with default values. */
	IntegratorErrorControlled();

	/*! Relative tolerance to use for error test. */
	double						m_relTol;

	/*! Stores absolute tolerance, vector of equation-specific values is used instead, when not empty. */
	double						m_absTol;

	/*! If not empty, equation-specific tolerances are used. */
	std::vector<double>			m_absTolVec;

	/*! Explicitely given time restriction for the time integration, time in seconds.
		The integrator must not exceed this time point (step beyond this time) during integration.
		Usually used if a simulation input value change is expected at m_stopTime, or some
		time-dependent quantity is not (yet) defined past this time point.
		If set to 0 no stop time is given (default behaviour).
	*/
	double						m_stopTime;
};


} // namespace SOLFRA

#endif // SOLFRA_INTEGRATORERRORCONTROLLED_H
