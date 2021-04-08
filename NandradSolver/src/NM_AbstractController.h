#ifndef AbstractControllerH
#define AbstractControllerH


namespace NANDRAD_MODEL {

/*! Defines the interface for an abstract controller.
	Given a setpoint and a process variable, the controller instance
	calculates controller error and controler output, normalized to [0,...,1].
	This interface can be called from any model that provides abstract quantities.
	\code
	//derived class from AbstractController
	PController controller;
	// set parameters
	// ...
	// new calculation step:
	// set current state and target value
	controller.m_currentState = temperatureDifference;
	controller.m_targetValue = setPointTemperatureDifference
	// calculate new controller output
	controller.updateControllerOutput();
	// retrieve new control value
	double massFlux = contoller.m_controllerOutput
	// ...
*/
class AbstractController {
public:
	AbstractController() {}

	/*! D'tor, definition is in NM_Controller.cpp. */
	virtual ~AbstractController();

	/*! Calculates controller output and controller error.*/
	virtual void updateControllerOutput() = 0;

	/*! Stores controller output.*/
	double				m_controllerOutput;
	/*! Stores internal conroller state.*/
	double				m_currentState;
	/*! Stores controller target.*/
	double				m_targetValue;
};

} // namespace NANDRAD_MODEL

#endif // AbstractControllerH
