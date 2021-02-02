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
		MT_StaticAdiabaticPipe,				// Keyword: StaticAdiabaticPipe			'Simple pipe at stationary flow conditions without heat exchange'
		MT_StaticPipe,						// Keyword: StaticPipe					'Simple pipe at stationary flow conditions with heat exchange'
		MT_DynamicAdiabaticPipe,			// Keyword: DynamicAdiabaticPipe		'Pipe with a discretized fluid volume, without heat exchange'
		MT_DynamicPipe,						// Keyword: DynamicPipe					'Pipe with a discretized fluid volume and heat exchange'
		MT_ConstantPressurePumpModel,		// Keyword: ConstantPressurePumpModel	'Pump with constant pressure'
		MT_HeatExchanger,					// Keyword: HeatExchanger				'Simple heat exchanger with given heat flux'

		// models below not supported yet

		MT_HeatPump,						// Keyword: HeatPump					'Heat pump'
		MT_GasBoiler,						// Keyword: GasBoiler					'Gas boiler'
		MT_ControlValve,					// Keyword: ControlValve				'Control valve'
		MT_WaterStorage,					// Keyword: WaterStorage				'Water storage'
		MT_ComponentConditionSystem,		// Keyword: ComponentConditionSystem	'Component conditioning system is a system for heating or cooling of components'
		MT_Radiator,						// Keyword: Radiator					'Radiator'
		MT_Mixer,							// Keyword: Mixer						'Mixer component'
		MT_FMU,								// Keyword: FMU							'Flow characteristics provided by FMU'
		NUM_MT
	};

	/*! Parameters for the component. */
	enum para_t {
		P_HydraulicDiameter,				// Keyword: HydraulicDiameter					[mm]	'Only used for pressure loss calculation with PressureLossCoefficient (NOT for pipes).'
		P_PressureLossCoefficient,			// Keyword: PressureLossCoefficient				[-]		'Pressure loss coefficient for the component (zeta-value).'
		P_ExternalHeatTransferCoefficient,	// Keyword: ExternalHeatTransferCoefficient		[W/m2K]	'External heat transfer coeffient for the outside boundary.'
		P_PressureHead,						// Keyword: PressureHead						[Pa]	'Pressure head form a pump.'
		P_PumpEfficiency,					// Keyword: PumpEfficiency						[---]	'Pump efficiency.'
		P_MotorEfficiency,					// Keyword: MotorEfficiency						[---]	'Motor efficiency for a pump.'
		P_Volume,							// Keyword: Volume								[m3]	'Water or air volume of the component.'
		P_COP,								// Keyword: COP									[-]		'Coefficient of performance of the component.'
		P_UAValue,							// Keyword: UAValue								[W/m2K]	'UA-Value of heat exchanger'
		P_PipeMaxDiscretizationWidth,		// Keyword: PipeMaxDiscretizationWidth			[m]		'Maximum width of discretized volumes in pipe'

// we can add those, once we know what to do with them

//		P_TemperatureTolerance,				// xKxeyword: TemperatureTolerance				[K]		'Temperature tolerance for e.g. thermostats.'
//		P_RatedHeatingCapacity,				// xKxeyword: RatedHeatingCapacity				[W]		'Rated heating capacity of the component.'
//		P_RatedCoolingCapacity,				// xKxeyword: RatedCoolingCapacity				[W]		'Rated Cooling capacity of the component.'
//		P_AuxiliaryPower,					// xKxeyword: AuxiliaryPower						[W]		'Auxiliary power of the component.'
//		P_ConvectiveFraction,				// xKxeyword: ConvectiveFraction					[---]	'Convective fraction for heating or cooling.'
//		P_ExternalSurfaceArea,				// xKxeyword: ExternalSurfaceArea					[m2]	'External surface area of the component.'

		NUM_P
	};


	enum HeatExchangeType {
		HT_TemperatureConstant,				// Keyword: TemperatureConstant					[-]		'Constant temperature'
		HT_HeatFluxConstant,				// Keyword: HeatFluxConstant					[-]		'Constant heat flux'
		HT_HeatFluxDataFile,				// Keyword: HeatFluxDataFile					[-]		'Heat flux from data file'
		HT_TemperatureDataFile,				// Keyword: TemperatureDataFile					[-]		'Temperature from data file'
		HT_HeatExchangeWithZoneTemperature,	// Keyword: HeatExchangeWithZoneTemperature		[-]		'Heat exchange with zone'
		HT_HeatExchangeWithFMUTemperature,	// Keyword: HeatExchangeWithFMUTemperature		[-]		'Heat exchange with FMU which requires temperature and provides heat flux'
		NUM_HT
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
	ModelType						m_modelType			= NUM_MT;						// XML:A:required

	/*! Type of interface to external data or model */
	HeatExchangeType				m_heatExchangeType = NUM_HT;						// XML:E

	/*! Parameters of the flow component. */
	IBK::Parameter					m_para[NUM_P];										// XML:E


	// *** STATIC FUNCTIONS ***

	static bool hasHeatExchange(const ModelType modelType) {
		switch (modelType) {
			case MT_StaticPipe:
			case MT_DynamicPipe:
			case MT_HeatPump:
			case MT_HeatExchanger:
			case MT_Radiator:
				return true;
			default:
				return false;
		}
	}


	/*! Needed for user interface, not for solver. */
	static std::vector<unsigned int> requiredParameter(const ModelType modelType){
		switch (modelType) {
			case MT_ConstantPressurePumpModel:
				return {P_PressureHead, P_PumpEfficiency, P_MotorEfficiency, P_Volume};
			case MT_HeatPump:
				return {P_PressureLossCoefficient, P_HydraulicDiameter, P_COP, P_Volume};
			case MT_HeatExchanger:
				return {P_PressureLossCoefficient, P_HydraulicDiameter, P_Volume};
			default:
				return {};
		}
	}

};

} // namespace NANDRAD

#endif // NANDRAD_HydraulicNetworkComponentH
