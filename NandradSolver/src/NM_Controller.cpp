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

#include "NM_Controller.h"


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

	// change controller output only if we are outside tolerance band
	if (errorValue > m_hysteresisBand) // too cold, turn heating on: example: setpoint = 23, Troom = 22 -> errorValue = 1 K
		m_nextControlValue = 1.0;
	else if(errorValue < -m_hysteresisBand) // too warm, turn heating off: example: setpoint = 23, Troom = 24 -> errorValue = -1 K
		m_nextControlValue = 0.0;
}


void DigitalHysteresisController::stepCompleted(double /*t*/) {
	m_controlValue = m_nextControlValue;
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


void PIController::stepCompleted(double t) {
	double dt = t - m_tLastStep;
	// trapozoid rule of integration
	m_errorValueIntegral += dt*0.5*(m_lastErrorValue + m_errorValue);
	m_tLastStep = t;
}


void PIController::resetErrorIntegral() {
	m_errorValueIntegral = 0;
}


} // namespace NANDRAD_MODEL
