#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "NANDRAD_Constants.h"
#include "NANDRAD_CodeGenMacros.h"

#include <IBK_Parameter.h>

namespace NANDRAD {

class Controller
{
public:

	NANDRAD_READWRITE
	NANDRAD_COMPARE_WITH_ID
	NANDRAD_COMP(Controller)

	Controller();

	enum Type {
		T_DigitalDirect,		// Keyword: DigitalDirect			'DigitalDirect'
		T_DigitalHysteresis,	// Keyword: DigitalHysteresis		'DigitalHysteresis'
		T_PController,			// Keyword: PController				'PController'
		T_PIController,			// Keyword: PIController			'PIController'
		NUM_T
	};

	enum para_t{
		P_Kp,
		P_Ki,
		P_Kd,
		P_HysteresisBand,
		NUM_P
	};

	IDType				m_id = INVALID_ID;							// XML:A:required

	Type				m_type = NUM_T;								// XML:E

	/*! if the controller error is below the tolerance,
	 * the result is interpreted as "accurate enough" and the controller output will not be changed */
	double				m_tolerance;								// XML:A

	IBK::Parameter		m_par[NUM_P];								// XML:E

};


}  // namespace NANDRAD

#endif // CONTROLLER_H
