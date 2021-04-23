#include "NM_Controller.h"

#include <NANDRAD_Controller.h>

namespace NANDRAD_MODEL {

AbstractController::~AbstractController() {
}

// *** Digital direct Controller ***

void DigitalDirectController::update(double errorValue) {
	AbstractController::update(errorValue);
	// calculate controller output
	// example: if below setpoint, turn heating on
	m_controlValue = (errorValue > 0 ) ? 1.0 : 0.0;
}


// *** Digital hysteresis Controller ***

void DigitalHysteresisController::update(double errorValue) {
	AbstractController::update(errorValue);

	m_controlValue = m_previousControlValue;
	// change controller output only if we are outside tolerance band
	if (errorValue > m_hysteresisBand) // too cold, turn heating on: example: setpoint = 23, Troom = 22 -> errorValue = 1 K
		m_controlValue = 1.0;
	else if(errorValue < -m_hysteresisBand) // too warm, turn heating off: example: setpoint = 23, Troom = 24 -> errorValue = -1 K
		m_controlValue = 0.0;
}

void DigitalHysteresisController::stepCompleted(double /*t*/) {
	m_previousControlValue = m_controlValue;
}


// *** P-Controller ***

void PController::update(double errorValue) {
	AbstractController::update(errorValue);
	m_controlValue = m_kP * errorValue;
}


// *** PI-Controller ***

void PIController::update(double errorValue) {
	AbstractController::update(errorValue);
	// calculate controller output
	m_controlValue = m_kP * errorValue + m_kI * m_errorValueIntegral;
}


} // namespace NANDRAD_MODEL
