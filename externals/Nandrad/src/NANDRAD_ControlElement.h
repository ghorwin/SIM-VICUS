#ifndef CONTROLELEMENT_H
#define CONTROLELEMENT_H

#include "NANDRAD_LinearSplineParameter.h"
#include "NANDRAD_Constants.h"

#include <IBK_Parameter.h>

namespace NANDRAD {

class Controller;

class ControlElement
{

public:

	NANDRAD_READWRITE

	ControlElement();

	/*! Checks for valid and required parameters (value ranges).
	*/
	void checkParameters(const std::vector<Controller> &controllers);

	enum ControlType{
		CT_ControlTemperatureDifference,	// Keyword: ControlTemperatureDifference	'ControlTemperatureDifference'
		CT_ControlMassFlow,					// Keyword: ControlMassFlow					'ControlMassFlow'
		CT_ControlZoneAirTemperature,		// Keyword: ControlZoneAirTemperature		'ControlZoneAirTemperature'
		NUM_CT
	};

	ControlType						m_controlType = NUM_CT;								// XML:A

	/*! reference to a controller (P, PI, ..) */
	IDType							m_controllerId = INVALID_ID;						// XML:A

	/*! the set point as fixed scalar value */
	IBK::Parameter					m_setPoint;											// XML:E

	/*! the set point as a schedule */
	std::string						m_setPointScheduleName;								// XML:E

	/*! used to cut the system input */
	IBK::Parameter					m_maximumSystemInput;								// XML:E


	// *** run time variables ***
	const NANDRAD::Controller		*m_controller = nullptr;
};

} // namespace NANDRAD

#endif // CONTROLELEMENT_H
