/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef NM_ControllerH
#define NM_ControllerH

#include "NM_AbstractController.h"
#include "NM_AbstractTimeDependency.h"


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
	/*! Start with signal 0. */
	DigitalHysteresisController();

	/*! Calculates controller signal/control value. */
	void update(double errorValue) override;

	/*! Stores state after a successful iteration step. */
	void stepCompleted(double) override;

	/*! Computes and returns serialization size in bytes. */
	std::size_t serializationSize() const override;

	/*! Stores control value at memory*/
	void serialize(void* & dataPtr) const override;

	/*! Restores control value from memory.*/
	void deserialize(void* & dataPtr) override;

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

	\warning This controller has a state m_errorValueIntegral. When
		restoring the FMI state, this controller state must be restored as well.
*/
class PIController: public AbstractController { // NO KEYWORDS
public:
	/*! Calculates controller signal/control value. */
	void update(double errorValue) override;

	/*! This function is called after each integration step and integrates the errorValue. */
	virtual void stepCompleted(double t) override;

	/*! Computes and returns serialization size in bytes. */
	std::size_t serializationSize() const override;

	/*! Stores control value at memory*/
	void serialize(void* & dataPtr) const override;

	/*! Restores control value from memory.*/
	void deserialize(void* & dataPtr) override;

	/*! sets error integral value to 0 (anti-windup), should be implemented for PI controllers */
	virtual void resetErrorIntegral() override;
	
	/*! P-term factor.*/
	double			m_kP = 1;
	/*! I-term factor.*/
	double			m_kI = 0.001;

	/*! Stores the error value integral.
		This is updated in stepCompleted();
	*/
	double			m_errorValueIntegral = 0;
	/*! Cached last error value, used in trapozoid rule when integrating error value. */
	double			m_lastErrorValue = 0;
	/*! Cached time of last stepCompleted call with m_lastErrorValue = e(m_tLastStep). */
	double			m_tLastStep = 0;
};

} // namespace NANDRAD_MODEL

#endif // NM_ControllerH
