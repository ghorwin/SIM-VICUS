#ifndef VICUS_NETWORKCONTROLLER_H
#define VICUS_NETWORKCONTROLLER_H

#include "VICUS_AbstractDBElement.h"
#include "VICUS_CodeGenMacros.h"

#include <NANDRAD_HydraulicNetworkControlElement.h>

#include <QColor>


namespace VICUS {

class NetworkController: public AbstractDBElement, public NANDRAD::HydraulicNetworkControlElement {

public:
	NetworkController();

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMP(NetworkController)

	/*! Checks if all parameters are valid. */
	bool isValid() const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;


	// *** PUBLIC MEMBER VARIABLES ***

	/*! Display name. */
	IBK::MultiLanguageString			m_displayNameML;									// XML:A

	/*! False color. */
	QColor								m_color;											// XML:A


	/*!
		*** Only for Code Generator ***
		*** NANDRAD::HydraulicNetworkComponent interface ***


	enum modelType_t {
		MT_Constant,					// Keyword: Constant				'Set points are given as constant parameters'
		MT_Scheduled,					// Keyword: Scheduled				'Scheduled set point values'
		NUM_MT
	};

	enum ControlledProperty {
		CP_TemperatureDifference,						// Keyword: TemperatureDifference						'Control temperature difference of this element '
		CP_TemperatureDifferenceOfFollowingElement,		// Keyword: TemperatureDifferenceOfFollowingElement		'Control temperature difference of the following element'
		CP_ThermostatValue,								// Keyword: ThermostatValue								'Control zone thermostat values'
		CP_MassFlux,									// Keyword: MassFlux									'Control mass flux'
		NUM_CP
	};

	enum ControllerType {
		CT_PController,			// Keyword: PController				'PController'
		CT_PIController,		// Keyword: PIController			'PIController'
		NUM_CT
	};

	enum para_t {
		P_Kp,								// Keyword: Kp								[---]	'Kp-parameter'
		P_Ki,								// Keyword: Ki								[---]	'Ki-parameter'
		P_Kd,								// Keyword: Kd								[---]	'Kd-parameter'
		P_TemperatureDifferenceSetpoint,	// Keyword: TemperatureDifferenceSetpoint	[K]		'Target temperature difference'
		P_MassFluxSetpoint,					// Keyword: MassFluxSetpoint				[kg/s]	'Target mass flux'
		NUM_P
	};

	enum References {
		ID_ThermostatZoneId,				// Keyword: ThermostatZoneId				[-]		'ID of zone containing thermostat'
		NUM_ID
	};

	IDType							m_id = NANDRAD::INVALID_ID;						// XML:A:required

	modelType_t						m_modelType;									// XML:A:required

	ControllerType					m_controllerType = NUM_CT;						// XML:A

	ControlledProperty				m_controlledProperty = NUM_CP;					// XML:A:required

	IDType							m_idReferences[NUM_ID];							// XML:E

	double							m_maximumControllerResultValue = 0;				// XML:E

	IBK::Parameter					m_para[NUM_P];									// XML:E

	*/

};



} // namespace VICUS

#endif // VICUS_NETWORKCONTROLLER_H
