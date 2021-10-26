#ifndef CONTROLELEMENT_H
#define CONTROLELEMENT_H

#include "NANDRAD_LinearSplineParameter.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_Zone.h"
#include "NANDRAD_HydraulicNetworkComponent.h"

#include <IBK_Parameter.h>

namespace NANDRAD {


/*! This class contains parameters for a controller that is used for network elements.
	A controller is actually a flow controller that impacts the way the flow elements-system function
	is evaluated, usually adding a flow resistance (e.g. value) to the element. The additional flow resistance
	can be controlled in different ways, as defined by ControlledProperty.
*/
class HydraulicNetworkControlElement {
public:

	NANDRAD_READWRITE
	NANDRAD_COMPARE_WITH_ID
	NANDRAD_COMP(HydraulicNetworkControlElement)

	/*! Checks for valid and required parameters (value ranges). */
	void checkParameters(const std::vector<NANDRAD::Zone> &zones) const;

	/*! Defines variability options for set point values. */
	enum ModelType {
		MT_Constant,					// Keyword: Constant				'Set points are given as constant parameters'
		/*! Generates 'TemperatureDifferenceSetpointSchedule' or 'MassFluxSetpointSchedule' depending on
			controlled property for each network element that uses this controller.
			InputRef-Typ: NetworkElement, id = id of network element that uses the component with this controller
		*/
		MT_Scheduled,					// Keyword: Scheduled				'Scheduled set point values'
		NUM_MT
	};

	/*! Controlled property used as signal input for the controller. */
	enum ControlledProperty {
		/*! Temperature difference is computed from pre-defined heat loss and compared against target temperature difference. */
		CP_TemperatureDifference,						// Keyword: TemperatureDifference						'Control temperature difference of this element '
		/*! Temperature difference of the following flow element is computed from outlet temperature of this element and outlet temperature of the following element.
			This temperature difference is compared against target temperature difference. */
		CP_TemperatureDifferenceOfFollowingElement,		// Keyword: TemperatureDifferenceOfFollowingElement		'Control temperature difference of the following element'
		/*! Thermostat heating/cooling control values determine whether valve is open or closed. */
		CP_ThermostatValue,								// Keyword: ThermostatValue								'Control zone thermostat values'
		/*! Try to achieve target mass flow in current element. */
		CP_MassFlux,									// Keyword: MassFlux									'Control mass flux'
		/*! Switch pump on or off depending on heat flux of following element */
		CP_PumpOperation,								// Keyword: PumpOperation								'Control pump operation depending on following element'
		NUM_CP
	};

	/*! Different model variants. */
	enum ControllerType {
		CT_PController,			// Keyword: PController				'PController'
		CT_PIController,		// Keyword: PIController			'PIController'
		CT_OnOffController,		// Keyword: OnOffController			'OnOffController'
		NUM_CT
	};

	/*! Model parameters. */
	enum para_t {
		P_Kp,								// Keyword: Kp								[---]	'Kp-parameter'
		P_Ki,								// Keyword: Ki								[---]	'Ki-parameter'
		P_Kd,								// Keyword: Kd								[---]	'Kd-parameter'
		P_TemperatureDifferenceSetpoint,	// Keyword: TemperatureDifferenceSetpoint	[K]		'Target temperature difference'
		P_MassFluxSetpoint,					// Keyword: MassFluxSetpoint				[kg/s]	'Target mass flux'
		P_HeatLossOfFollowingElementThreshold,	// Keyword: HeatLossOfFollowingElementThreshold		[W]		'Threshold value for PumpOperation property when OnOffController is used'
		NUM_P
	};

	/*! Integer/whole number parameters. */
	enum References {
		/*! Id of zone whose thermostat is used for control: only for controlled property 'ThermostatValue'. */
		ID_ThermostatZoneId,				// Keyword: ThermostatZoneId				[-]		'ID of zone containing thermostat'
		NUM_ID
	};

	IDType							m_id = NANDRAD::INVALID_ID;						// XML:A:required

	/*! Determines variability of setpoint parameters. */
	ModelType						m_modelType = NUM_MT;							// XML:A:required

	/*! Controller type (P, PI, ...) */
	ControllerType					m_controllerType = NUM_CT;						// XML:A

	/*! property which shall be controlled (temperature difference, ...) */
	ControlledProperty				m_controlledProperty = NUM_CP;					// XML:A:required

	/*! Integer/ID reference parameters. */
	IDType							m_idReferences[NUM_ID];							// XML:E

	/*! Used to cut the system input, if this is zero, it will not be considered	*/
	double							m_maximumControllerResultValue = 0;				// XML:E

	/*! Controller parameters. */
	IBK::Parameter					m_para[NUM_P];									// XML:E

	static std::vector<ControlledProperty> availableControlledProperties(const HydraulicNetworkComponent::ModelType modelType);
};

} // namespace NANDRAD

#endif // CONTROLELEMENT_H
