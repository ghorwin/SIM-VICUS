#ifndef NM_ControllerH
#define NM_ControllerH

#include "NM_AbstractController.h"
#include "NM_AbstractTransientController.h"
#include "NM_AbstractTimeDependency.h"

namespace NANDRAD {
	class Controller;
};

namespace NANDRAD_MODEL {

/*! Defines a digital direct controller instance.
	\code
		// example for heating control
		errorValue = setpoint - current_temperature;
		// turn heating on when below setpoint
		controlValue = errorValue < 0 : 1 : 0;
	\endcode
*/
class DigitalDirectController: public AbstractController { // NO KEYWORDS
public:
	/*! Calculates controller signal/control value. */
	void update(double errorValue) override;
};


/*! Defines a digital controller instance with hystersis.
	The controller logic switches at +hysteresisBand and -hysteresisBand.
	\code
		// hysteresis band set to 2 K (+2K ... -2K, total 4 K distance between switch points)
		controller.m_hysteresisBand = 2;

		controller.update(-2.3); // below hysteresis band, controller will be turned on
		controller.update(-1.3); // inside hysteresis band, controller stays on
		controller.update(2.1);  // above hysteresis band, controller is turned off
	\endcode
	Current on/off state is updated after each integration step.

	Since controller state change only takes effect in the _next_ step, this is effectively
	an explicit controller, and rapid oscillations and instability is only prevented due to the dead band.
*/
class DigitalHysteresisController: public AbstractController { // NO KEYWORDS
public:
	/*! Calculates controller signal/control value. */
	void update(double errorValue) override;

	/*! Stores state after a successful iteration step. */
	void stepCompleted(double) override;

	/*! Tolerance band: must be set from outside. */
	double			m_hysteresisBand;
	/*! Controller signal to be used for next step (updated in update()). */
	double			m_nextControlValue = 0.0;
};


/*! Defines a proportional controller instance.
	\code
		controlValue = errorValue*kP;
	\endcode
*/
class PController: public AbstractController { // NO KEYWORDS
public:
	/*! Calculates controller signal/control value. */
	void update(double errorValue) override;

	/*! P-term factor (1 by default, just pass through of signal). */
	double			m_kP = 1;
};


/*! Defines a PI controller instance.
	\code
		controlValue = errorValue*kP + errorValueIntegral*kI;
	\endcode
*/
class PIController: public AbstractTransientController { // NO KEYWORDS
public:
	/*! Calculates controller signal/control value. */
	void update(double errorValue) override;

	/*! P-term factor.*/
	double			m_kP = 1;
	/*! I-term factor.*/
	double			m_kI = 1;
};

} // namespace NANDRAD_MODEL

#endif // NM_ControllerH
