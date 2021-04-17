#include "NM_Controller.h"

#include <NANDRAD_Controller.h>

namespace NANDRAD_MODEL {


// *** Digital direct Controller ***

DigitalDirectController::DigitalDirectController() {
}


void DigitalDirectController::updateControllerOutput()
{
	// calculate controller output
	if(m_currentState < m_targetValue )
		m_controllerOutput = 1.0;
	else
		m_controllerOutput = 0.0;
}


// *** Digital hysteresis Controller ***

DigitalHysteresisController::DigitalHysteresisController() {
}


void DigitalHysteresisController::updateControllerOutput()
{
	m_controllerOutput = m_previousControllerOutput;

	// change controller output only if we are outside tolerance band
	if(m_currentState >= m_targetValue + m_hysteresisBand)
		m_controllerOutput = 0.0;
	else if(m_currentState <= m_targetValue - m_hysteresisBand)
		m_controllerOutput = 1.0;
	else if(m_previousControllerOutput == 0.0 && m_currentState > m_targetValue - m_hysteresisBand )
		m_controllerOutput = 0.0;
	else //(m_previousControllerOutput == 1.0 && m_currentState < m_targetValue + m_hysteresisBand )
		m_controllerOutput = 1.0;
}

void DigitalHysteresisController::stepCompleted(double /*t*/) {
	m_previousControllerOutput = m_controllerOutput;
}


// *** P-Controller ***

PController::PController(const NANDRAD::Controller &controller) {
	// copy kP parameter
	m_kP = controller.m_para[NANDRAD::Controller::P_Kp].value;
}


void PController::updateControllerOutput()
{
	// calculate controller output
	m_controllerOutput = m_kP * (m_targetValue - m_currentState);
}


// *** PI-Controller ***

PIController::PIController(const NANDRAD::Controller &controller) {
	// copy kP parameter
	m_kP = controller.m_para[NANDRAD::Controller::P_Kp].value;
	// copy kI parameter
	m_kI = controller.m_para[NANDRAD::Controller::P_Ki].value;
}


void PIController::updateControllerOutput()
{
	// calculate controller output
	m_controllerOutput = m_kP * (m_targetValue - m_currentState) + m_kI * m_controllerErrorIntegral;
}



} // namespace NANDRAD_MODEL
