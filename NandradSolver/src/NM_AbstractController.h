#ifndef NM_AbstractControllerH
#define NM_AbstractControllerH

namespace NANDRAD_MODEL {

/*! Defines the interface for an abstract controller.
	This interface can be called from any model that provides abstract quantities.
	\code
		//derived class from AbstractController
		PController controller;
		// set parameters
		// ...
		// new calculation step:
		// compute error value as difference between target and current process value
		errorValue = setPointTemperatureDifference - temperatureDifference;
		// calculate new controller output
		controller.update(errorValue);
		// retrieve new control value
		double massFlux = contoller.m_controlValue
		// ...
	\endcode
*/
class AbstractController {
public:
	/*! D'tor, definition is in NM_Controller.cpp. */
	virtual ~AbstractController();

	/*! Calculates controller signal/control value. This function must be re-implemented in derived classes.
		Default implementation caches the errorValue. Call the base class implementation from derived classes.
		\param errorValue This is the input error value, i.e. the difference between
				desired setpoint and current process variable.
	*/
	virtual void update(double errorValue) { m_errorValue = errorValue; }

	/*! Stores result of controller calculation, updated in each call to update(). */
	double				m_controlValue = 888;
	/*! Cache of error value updated in last call to update(). */
	double				m_errorValue = 777;
};

} // namespace NANDRAD_MODEL

#endif // NM_AbstractControllerH
