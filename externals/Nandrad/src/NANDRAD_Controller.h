#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "NANDRAD_Constants.h"
#include "NANDRAD_CodeGenMacros.h"

#include <IBK_Parameter.h>

namespace NANDRAD {

class Controller
{
public:
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
		NUM_P
	};


	Controller();

	NANDRAD_READWRITE
	NANDRAD_COMPARE_WITH_ID
	NANDRAD_COMP(Controller)

	/*! Checks for valid and required parameters (value ranges).
	*/
	void checkParameters();

	IDType				m_id = INVALID_ID;							// XML:A:required

	Type				m_type = NUM_T;								// XML:E

	IBK::Parameter		m_par[NUM_P];								// XML:E

};


}  // namespace NANDRAD

#endif // CONTROLLER_H
