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

#include <algorithm>

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

DigitalHysteresisController::DigitalHysteresisController() {
	m_controlValue = 0.0;
}

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


std::size_t DigitalHysteresisController::serializationSize() const {
	// controlValue
	return sizeof(double);
}


void DigitalHysteresisController::serialize(void *& dataPtr) const {
	// cache controlValue for hysteresis
	*(double*)dataPtr = m_controlValue;
	dataPtr = (char*)dataPtr + sizeof(double);
}


void DigitalHysteresisController::deserialize(void *& dataPtr) {
	// update cached controlValue
	m_controlValue = *(double*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(double);
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
	// store error value and time point
	m_lastErrorValue = m_errorValue;
}


std::size_t PIController::serializationSize() const {
	// lastTimeStep, lastErrorValue, errorValueIntegral
	return 3 * sizeof (double);
}


void PIController::serialize(void *& dataPtr) const {
	// cache last step and error value for integration
	*(double*)dataPtr = m_tLastStep;
	dataPtr = (char*)dataPtr + sizeof(double);
	*(double*)dataPtr = m_lastErrorValue;
	dataPtr = (char*)dataPtr + sizeof(double);
	// cache integral value
	*(double*)dataPtr = m_errorValueIntegral;
	dataPtr = (char*)dataPtr + sizeof(double);
}


void PIController::deserialize(void *& dataPtr) {
	// update cached last step
	m_tLastStep = *(double*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(double);
	// update cached last error value
	m_lastErrorValue = *(double*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(double);
	// update cached error integral
	m_errorValueIntegral = *(double*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(double);
}


void PIController::resetErrorIntegral() {
	m_errorValueIntegral = 0;
}




// *** PID-Controller ***

void PIDController::update(double errorValue) {
	AbstractController::update(errorValue);
	// calculate controller output
	m_controlValue = m_kP * errorValue + m_kI * m_errorValueIntegral + m_kD * (errorValue - m_lastErrorValue) / m_timeStep;
}


void PIDController::stepCompleted(double t) {
	// trapozoid rule of integration
	m_errorValueIntegral += m_timeStep*0.5*(m_lastErrorValue + m_errorValue);
	m_tLastStep = t;
	// store error value and time point
	m_lastErrorValue = m_errorValue;
}


void PIDController::setTime(double t)
{
	m_timeStep = t - m_tLastStep;
}


std::size_t PIDController::serializationSize() const {
	// lastTimeStep, lastErrorValue, errorValueIntegral
	return 3 * sizeof (double);
}


void PIDController::serialize(void *& dataPtr) const {
	// cache last step and error value for integration
	*(double*)dataPtr = m_tLastStep;
	dataPtr = (char*)dataPtr + sizeof(double);
	*(double*)dataPtr = m_lastErrorValue;
	dataPtr = (char*)dataPtr + sizeof(double);
	// cache integral value
	*(double*)dataPtr = m_errorValueIntegral;
	dataPtr = (char*)dataPtr + sizeof(double);
}


void PIDController::deserialize(void *& dataPtr) {
	// update cached last step
	m_tLastStep = *(double*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(double);
	// update cached last error value
	m_lastErrorValue = *(double*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(double);
	// update cached error integral
	m_errorValueIntegral = *(double*)dataPtr;
	dataPtr = (char*)dataPtr + sizeof(double);
}


void PIDController::resetErrorIntegral() {
	m_errorValueIntegral = 0;
}


} // namespace NANDRAD_MODEL
