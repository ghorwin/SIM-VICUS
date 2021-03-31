#ifndef CONTROLELEMENT_H
#define CONTROLELEMENT_H

#include "NANDRAD_LinearSplineParameter.h"
#include "NANDRAD_Constants.h"

#include <IBK_Parameter.h>

namespace NANDRAD {

class ControlElement
{

public:

	NANDRAD_READWRITE

	ControlElement();

	enum ControlType{
		CT_ControlTemperatureDifference,	// Keyword: ControlTemperatureDifference	'ControlTemperatureDifference'
		CT_ControlMassFlow,					// Keyword: ControlMassFlow					'ControlMassFlow'
		CT_ControlZoneAirTemperature,		// Keyword: ControlZoneAirTemperature		'ControlZoneAirTemperature'
		NUM_CT
	};

	ControlType						m_controlType = NUM_CT;								// XML:A:required

	/*! reference to a controller (P, PI, ..) */
	IDType							m_controllerId = INVALID_ID;						// XML:A

	/*! the set point as fixed scalar value */
	IBK::Parameter					m_setPoint;											// XML:E

	/*! the set point as a linear spline */
	NANDRAD::LinearSplineParameter	m_setPointSpline;									// XML:E

	/*! the set point as a schedule */
	std::string						m_setPointScheduleName;								// XML:E

	/*! used to normalize the controller error */
	IBK::Parameter					m_maximumControllerError;							// XML:E

	/*! used to interpret the controller output (0...1) as a system input */
	IBK::Parameter					m_maximumSystemInput;								// XML:E

};

} // namespace NANDRAD

#endif // CONTROLELEMENT_H
