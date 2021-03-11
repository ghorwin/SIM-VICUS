#ifndef NANDRAD_HydraulicNetworkComponentH
#define NANDRAD_HydraulicNetworkComponentH

#include <IBK_Parameter.h>

#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Constants.h"


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
		MT_HeatPumpIdealCarnot,				// Keyword: HeatPumpIdealCarnot			'Heat pump with unlimited heating power and constant carnot efficiency'

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
		P_HydraulicDiameter,				// Keyword: HydraulicDiameter					[mm]	'Only used for pressure loss calculation with PressureLossCoefficient (NOT for pipes)'
		P_PressureLossCoefficient,			// Keyword: PressureLossCoefficient				[-]		'Pressure loss coefficient for the component (zeta-value)'
		P_PressureHead,						// Keyword: PressureHead						[Pa]	'Pressure head form a pump'
		P_PumpEfficiency,					// Keyword: PumpEfficiency						[---]	'Pump efficiency'
		P_Volume,							// Keyword: Volume								[m3]	'Water or air volume of the component'
		P_PipeMaxDiscretizationWidth,		// Keyword: PipeMaxDiscretizationWidth			[m]		'Maximum width of discretized volumes in pipe'
		P_CarnotEfficiency,					// Keyword: CarnotEfficiency					[---]	'Carnot efficiency'
		P_CondenserMeanTemperature,			// Keyword: CondenserMeanTemperature			[C]		'Mean fluid temperature in condenser'

// we can add those, once we know what to do with them

//		P_TemperatureTolerance,				// xKxeyword: TemperatureTolerance				[K]		'Temperature tolerance for e.g. thermostats'
//		P_RatedHeatingCapacity,				// xKxeyword: RatedHeatingCapacity				[W]		'Rated heating capacity of the component'
//		P_RatedCoolingCapacity,				// xKxeyword: RatedCoolingCapacity				[W]		'Rated Cooling capacity of the component'
//		P_AuxiliaryPower,					// xKxeyword: AuxiliaryPower						[W]		'Auxiliary power of the component'
//		P_ConvectiveFraction,				// xKxeyword: ConvectiveFraction					[---]	'Convective fraction for heating or cooling'
//		P_ExternalSurfaceArea,				// xKxeyword: ExternalSurfaceArea					[m2]	'External surface area of the component'

		NUM_P
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

	/*! Parameters of the flow component. */
	IBK::Parameter					m_para[NUM_P];										// XML:E


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
