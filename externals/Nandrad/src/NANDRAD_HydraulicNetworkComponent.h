#ifndef NANDRAD_HydraulicNetworkComponentH
#define NANDRAD_HydraulicNetworkComponentH

#include <IBK_Parameter.h>

#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_LinearSplineParameter.h"


namespace NANDRAD {

/*! Contain dataset for a hydraulic component for the network.

*/
class HydraulicNetworkComponent {
public:

	/*! The various types (equations) of the hydraulic component. */
	enum ModelType {
		MT_SimplePipe,						// Keyword: SimplePipe					'Pipe with a single fluid volume and with heat exchange'
		MT_DynamicPipe,						// Keyword: DynamicPipe					'Pipe with a discretized fluid volume and heat exchange'
		MT_ConstantPressurePump,			// Keyword: ConstantPressurePump		'Pump with constant pressure'
		MT_HeatExchanger,					// Keyword: HeatExchanger				'Simple heat exchanger with given heat flux'
		MT_HeatPumpIdealCarnot,				// Keyword: HeatPumpIdealCarnot			'Heat pump with variable heating power based on carnot efficiency'
		MT_HeatPumpReal,					// Keyword: HeatPumpReal				'On-off-type heat pump with based on manufacturer data sheet'

		// models below not supported yet

//		MT_GasBoiler,						// xKeyword: GasBoiler					'Gas boiler'
//		MT_ControlValve,					// xKeyword: ControlValve				'Control valve'
//		MT_WaterStorage,					// xKeyword: WaterStorage				'Water storage'
//		MT_ComponentConditionSystem,		// xKeyword: ComponentConditionSystem	'Component conditioning system is a system for heating or cooling of components'
//		MT_Radiator,						// xKeyword: Radiator					'Radiator'
//		MT_Mixer,							// xKeyword: Mixer						'Mixer component'
//		MT_FMU,								// xKeyword: FMU						'Flow characteristics provided by FMU'
		NUM_MT
	};

	/*! Parameters for the component. */
	enum para_t {
		P_HydraulicDiameter,					// Keyword: HydraulicDiameter					[mm]	'Only used for pressure loss calculation with PressureLossCoefficient (NOT for pipes)'
		P_PressureLossCoefficient,				// Keyword: PressureLossCoefficient				[-]		'Pressure loss coefficient for the component (zeta-value)'
		P_PressureHead,							// Keyword: PressureHead						[Pa]	'Pressure head form a pump'
		P_PumpEfficiency,						// Keyword: PumpEfficiency						[---]	'Pump efficiency'
		P_Volume,								// Keyword: Volume								[m3]	'Water or air volume of the component'
		P_PipeMaxDiscretizationWidth,			// Keyword: PipeMaxDiscretizationWidth			[m]		'Maximum width of discretized volumes in pipe'
		P_CarnotEfficiency,						// Keyword: CarnotEfficiency					[---]	'Carnot efficiency eta'
		P_MaximumHeatHeatingPower,				// Keyword: MaximumHeatHeatingPower				[W]		'Maximum heating power'
		P_CondenserNominalTemperatureDifference,	// Keyword: CondenserNominalTemperatureDifference	[C]		'Nominal temperature difference at condenser'
		P_EvaporatorNominalTemperatureDifference,	// Keyword: EvaporatorNominalTemperatureDifference	[C]		'Nominal temperature difference at evaporator'
		NUM_P
	};

	/*! Defines which side of heat pump is part of the network */
	enum HeatPumpIntegration {
		HP_SupplySide,						// Keyword: SupplySide				'The network is connected to the hot side (supply) of the heat pump'
		HP_SourceSide,						// Keyword: SourceSide				'The network is connected to the cold side (source) of the heat pump'
		HP_SupplyAndSourceSide,				// Keyword: SupplyAndSourceSide		'Two networks are connected, one to the cold side, the other to the hot side of the heat pump'
		NUM_HP
	};

	/*! Spline parameter as functions of time with the implied assumption that
		t = 0 means begin of simulation.
		TODO Andreas + Anne + Hauke
	*/
	enum splinePara_t {
		SPL_CondenserOutletSetPoint,		// Keyword: CondenserOutletSetPoint				[C]		'Set point temperature for condenser outlet'
		SPL_CondenserMeanTemperature,		// Keyword: CondenserMeanTemperature			[C]		'Mean fluid temperature in condenser'
		SPL_EvaporatorMeanTemperature,		// Keyword: EvaporatorMeanTemperature			[C]		'Mean fluid temperature in evaporator'
		SPL_HeatPumpControlSignal,			// Keyword: HeatPumpControlSignal				[---]	'Digital control signal (on/off) for heat pump'
		NUM_SPL
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE
	NANDRAD_COMPARE_WITH_ID
	NANDRAD_COMP(HydraulicNetworkComponent)

	/*! Compares two component definitions by parameters only, without comparing ID. */
	bool sameParametersAs(const HydraulicNetworkComponent & other) const;

	/*! Checks for valid and required parameters (value ranges).
		\param networkModelType Type of network calculation model (HydraulicNetwork::ModelType).
	*/
	void checkParameters(int networkModelType) const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID for this component. */
	unsigned int					m_id			= NANDRAD::INVALID_ID;				// XML:A:required

	/*! Display name. */
	std::string						m_displayName;										// XML:A

	/*! Model type. */
	ModelType						m_modelType		= NUM_MT;							// XML:A:required

	/*! Defines which side of heat pump is part of the network */
	HeatPumpIntegration				m_heatPumpIntegration = NUM_HP;						// XML:E

	/*! Parameters of the flow component. */
	IBK::Parameter					m_para[NUM_P];										// XML:E

	/*! Time-series of parameter (can be spline or tsv-file). */
	LinearSplineParameter			m_splPara[NUM_SPL];									// XML:E

	// *** STATIC FUNCTIONS ***

	/*! Needed both in user interface and for valid parameter checking in solver.
		\param networkModelType Identifies network model (HydraulicNetwork::ModelType).
	*/
	static std::vector<unsigned int> requiredParameter(const ModelType modelType, int networkModelType);

	/*! Helper function that implements specific rules for testing a single parameter.
		This is useful if the same parameter is used by several models and we want to avoid implementing
		the same checking rule multiple times.
	*/
	static void checkModelParameter(const IBK::Parameter &para, const unsigned int numPara);

};

} // namespace NANDRAD

#endif // NANDRAD_HydraulicNetworkComponentH
