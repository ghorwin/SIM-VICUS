#ifndef AbstractTransientControllerH
#define AbstractTransientControllerH

#include "NM_AbstractController.h"

namespace NANDRAD_MODEL {

/*! Defines the interface for an abstract controller with integral part.
	This class essentially adds two functions that can are used by time integrators to add time integration
	functionality.
*/
class AbstractTransientController: public AbstractController {
public:
	/*! Sets the new controller error integral so that it can be used in derived classes control value
		calculation functions.
	*/
	void setY(const double *y) { m_errorValueIntegral = y[0]; }

	/*! The error value is the time-derivative of the integral value. */
	void ydot(double *ydot) { ydot[0] = m_errorValue; }

	/*! Cached error value integral. */
	double				m_errorValueIntegral;
};

} // namespace NANDRAD_MODEL

#endif // AbstractTransientControllerH
