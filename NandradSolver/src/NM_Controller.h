#ifndef ControllerH
#define ControllerH

#include "NM_AbstractController.h"
#include "NM_AbstractTransientController.h"

namespace NANDRAD {
	class Controller;
};

namespace NANDRAD_MODEL {

/*! Defines a P controller instance.
*/
class PController: public AbstractController {
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
class PIController: public AbstractTransientController {
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

#endif // ControllerH
