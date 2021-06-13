#ifndef NETWORKCONTROLLER_H
#define NETWORKCONTROLLER_H

#include "VICUS_Constants.h"
#include "VICUS_CodeGenMacros.h"

#include <IBK_Parameter.h>


namespace VICUS {


class NetworkController
{
public:
	NetworkController();

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID
	VICUS_COMP(NetworkController)

	/*! Checks if all parameters are valid. */
	bool isValid() const;

	/*! Availabel control types. */
	enum ControlledProperty {
		CP_TemperatureDifference,			// Keyword: TemperatureDifference	'Control the temperature difference at the flow element'
		CP_MassFlow,						// Keyword: MassFlow				'Control the mass flow of the flow element'
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
		P_Kp,					// Keyword: Kp		[---]			'Proportional value of controller'
		P_Ki,					// Keyword: Ki		[---]			'Integral value of controller'
		P_Kd,					// Keyword: Kd		[---]			'Differential value of controller'
		NUM_P
	};

	unsigned int					m_id = VICUS::INVALID_ID;						// XML:A:required

	/*! Controller type (P, PI, ...) */
	ControllerType					m_controllerType = NUM_CT;						// XML:A:required

	/*! property which shall be controlled (temperature difference, ...) */
	ControlledProperty				m_controlledProperty = NUM_CP;					// XML:A:required

	/*! Set point as fixed scalar value. */
	IBK::Parameter					m_setPoint;										// XML:E

	/*! Used to cut the system input, if this is a negative value, it will not be considered	*/
	double							m_maximumControllerResultValue = -999;			// XML:E

	/*! Controller parameters. */
	IBK::Parameter					m_para[NUM_P];									// XML:E

	/*! Display name. */
	IBK::MultiLanguageString		m_displayName;									// XML:A

};


} // namespace VICUS


#endif // NETWORKCONTROLLER_H
