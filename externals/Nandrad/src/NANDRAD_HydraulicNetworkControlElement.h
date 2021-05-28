#ifndef CONTROLELEMENT_H
#define CONTROLELEMENT_H

#include "NANDRAD_LinearSplineParameter.h"
#include "NANDRAD_Constants.h"

#include <IBK_Parameter.h>

namespace NANDRAD {


/*! This class contains parameters for a controller that is used for network elements.
	A controller is actually a flow controller that impacts the way the flow elements-system function
	is evaluated, usually adding a flow resistance (e.g. value) to the element. The additional flow resistance
	can be controlled in different ways, as defined by ControlType.
*/
class HydraulicNetworkControlElement {
public:

	NANDRAD_READWRITE
	NANDRAD_COMPARE_WITH_ID
	NANDRAD_COMP(HydraulicNetworkControlElement)

	/*! Checks for valid and required parameters (value ranges). */
	void checkParameters();

	/*! Availabel control types. */
	enum ControlledProperty {
		CP_TemperatureDifference,			// Keyword: TemperatureDifference	'TemperatureDifference'
		CP_MassFlow,						// Keyword: MassFlow				'MassFlow'
		NUM_CP
	};

	/*! Different model variants. */
	enum ControllerType {
		CT_PController,			// Keyword: PController				'PController'
		CT_PIController,		// Keyword: PIController			'PIController'
		NUM_CT
	};

	/*! Model parameters. */
	enum para_t {
		P_Kp,					// Keyword: Kp					[---]	'Kp-parameter'
		P_Ki,					// Keyword: Ki					[---]	'Ki-parameter'
		P_Kd,					// Keyword: Kd					[---]	'Kd-parameter'
		NUM_P
	};

	IDType							m_id = NANDRAD::INVALID_ID;						// XML:A:required

	/*! Controller type (P, PI, ...) */
	ControllerType					m_controllerType = NUM_CT;						// XML:A:required

	/*! property which shall be controlled (temperature difference, ...) */
	ControlledProperty				m_controlledProperty = NUM_CP;					// XML:A:required

	/*! Set point as fixed scalar value. */
	IBK::Parameter					m_setPoint;										// XML:E

	/*! Used to cut the system input, if this is zero, it will not be considered	*/
	double							m_maximumControllerResultValue = 0;				// XML:E

	/*! Controller parameters. */
	IBK::Parameter					m_para[NUM_P];									// XML:E

};

} // namespace NANDRAD

#endif // CONTROLELEMENT_H
