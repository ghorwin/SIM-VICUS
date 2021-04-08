#include "NM_Controller.h"

#include <NANDRAD_Controller.h>

namespace NANDRAD_MODEL {

// *** P-Controller ***

PController::PController(const NANDRAD::Controller &controller) {
	// copy kP parameter
	m_kP = controller.m_par[NANDRAD::Controller::P_Kp].value;
}


void PController::updateControllerOutput()
{
	// calculate controller output
	m_controllerOutput = m_kP * (m_targetValue - m_currentState);
	// normalize
	m_controllerOutput = std::min(std::max(m_controllerOutput, 0.0), 1.0);
}


// *** PI-Controller ***

PIController::PIController(const NANDRAD::Controller &controller) {
	// copy kP parameter
	m_kP = controller.m_par[NANDRAD::Controller::P_Kp].value;
	// copy kI parameter
	m_kI = controller.m_par[NANDRAD::Controller::P_Ki].value;
}


void PIController::updateControllerOutput()
{
	// calculate controller output
	m_controllerOutput = m_kP * (m_targetValue - m_currentState) + m_kI * m_controllerErrorIntegral;
	// normalize
	m_controllerOutput = std::min(std::max(m_controllerOutput, 0.0), 1.0);
}



} // namespace NANDRAD_MODEL
