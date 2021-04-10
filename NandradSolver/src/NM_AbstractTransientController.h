#ifndef AbstractTransientControllerH
#define AbstractTransientControllerH

#include "NM_AbstractController.h"

namespace NANDRAD_MODEL {

/*! Defines the interface for an abstract controller with integral part.
	The controller requests an control error as time integration
	quantity.
	\code
	//derived class from AbstractTransientController
	PIController controller;
	// set parameters
	// ...
	// set current integration value
	controller.setY(m_y[offset]);
	// ...
	// set current state and target value
	// ...
	controller.m_currentState = temperatureDifference;
	controller.m_targetValue = setPointTemperatureDifference
	// calculate new controller output
	controller.updateControllerOutput();
	// retrieve new control value
	double massFlux = contoller.m_controllerOutput
	// ...
	// retrieve derivative
	constroller.ydot(m_ydot[offset]);
*/
class AbstractTransientController: public AbstractController {
public:
	AbstractTransientController() {}

	/*! Calculates controller output and controller error.*/
	virtual void setY(const double *y) { m_controllerErrorIntegral = y[0]; }

	/*! Calculates controller output and controller error.*/
	virtual void ydot(double *ydot) { ydot[0] = m_targetValue - m_currentState; }

	/*! Stores controller target.*/
	double				m_controllerErrorIntegral;
};

} // namespace NANDRAD_MODEL

#endif // AbstractTransientControllerH
