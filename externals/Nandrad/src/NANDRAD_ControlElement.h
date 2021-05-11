#ifndef CONTROLELEMENT_H
#define CONTROLELEMENT_H

#include "NANDRAD_LinearSplineParameter.h"
#include "NANDRAD_Constants.h"

#include <IBK_Parameter.h>

namespace NANDRAD {

class Controller;

/*! This class contains parameters for a controller that is used for network elements. */
class ControlElement {
public:

	NANDRAD_READWRITE

	/*! Checks for valid and required parameters (value ranges). */
	void checkParameters(const std::vector<Controller> &controllers);

	/*! Availabel control types. */
	enum ControlType {
		CT_ControlTemperatureDifference,	// Keyword: ControlTemperatureDifference	'ControlTemperatureDifference'
		CT_ControlMassFlow,					// Keyword: ControlMassFlow					'ControlMassFlow'
		CT_ControlZoneAirTemperature,		// Keyword: ControlZoneAirTemperature		'ControlZoneAirTemperature'
		NUM_CT
	};

	ControlType						m_controlType = NUM_CT;								// XML:A

	/*! Reference to a controller (P, PI, ..) */
	IDType							m_controllerId = INVALID_ID;						// XML:A

	/*! Set point as fixed scalar value. */
	IBK::Parameter					m_setPoint;											// XML:E

	/*! Set point as a schedule (TODO: refactor to select setpoint via modelType). */
	std::string						m_setPointScheduleName;								// XML:E

	/*! Used to cut the system input. */
	double							m_maximumControllerResultValue;						// XML:E


	// *** run time variables ***
	const NANDRAD::Controller		*m_controller = nullptr;
};

} // namespace NANDRAD

#endif // CONTROLELEMENT_H
