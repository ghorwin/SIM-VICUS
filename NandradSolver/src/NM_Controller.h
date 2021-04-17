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
*/
class DigitalDirectController: public AbstractController { // NO KEYWORDS
public:
	DigitalDirectController();

	/*! D'tor, definition is in NM_Controller.cpp. */
	virtual ~DigitalDirectController() { }

	/*! Calculates controller output and controller error.*/
	virtual void updateControllerOutput();
};


/*! Defines a digital controller instance with hystersis.
*/
class DigitalHysteresisController: public AbstractController, public AbstractTimeDependency { // NO KEYWORDS
public:
	DigitalHysteresisController();

	/*! D'tor, definition is in NM_Controller.cpp. */
	virtual ~DigitalHysteresisController() { }

	/*! Calculates controller output and controller error.*/
	virtual void updateControllerOutput();

	/*! Does nothing.*/
	virtual int setTime(double /*t*/) { return 0;}

	/*! Stores state after a successful iteration step.
	*/
	virtual void stepCompleted(double t);

	/*! Tolerance band: must be set from outside.*/
	double			m_hysteresisBand;
	/*! Controller signal of the previos time step.*/
	double			m_previousControllerOutput = 0.0;
};


/*! Defines a P controller instance.
*/
class PController: public AbstractController { // NO KEYWORDS
public:
	PController(const NANDRAD::Controller &controller);

	/*! D'tor, definition is in NM_Controller.cpp. */
	virtual ~PController() { }

	/*! Calculates controller output and controller error.*/
	virtual void updateControllerOutput();

	/*! P-controller, slope.*/
	double			m_kP;
};


/*! Defines a PI controller instance.
*/
class PIController: public AbstractTransientController { // NO KEYWORDS
public:
	PIController(const NANDRAD::Controller &controller);

	/*! D'tor, definition is in NM_Controller.cpp. */
	virtual ~PIController() { }

	/*! Calculates controller output and controller error.*/
	virtual void updateControllerOutput();

	/*! P-controller, slope.*/
	double			m_kP;
	/*! P-controller, integral slop.*/
	double			m_kI;
};

} // namespace NANDRAD_MODEL

#endif // NM_ControllerH
