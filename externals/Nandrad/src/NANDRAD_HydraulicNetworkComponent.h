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
	enum modelType_t {
		MT_StaticAdiabaticPipe,				// Keyword: StaticAdiabaticPipe			'Simple pipe at stationary flow conditions without heat exchange'
		MT_StaticPipe,						// Keyword: StaticPipe					'Simple pipe at stationary flow conditions with heat exchange'
		MT_DynamicAdiabaticPipe,			// Keyword: DynamicAdiabaticPipe		'Pipe with a discretized fluid volume, without heat exchange'
		MT_DynamicPipe,						// Keyword: DynamicPipe					'Pipe with a discretized fluid volume and heat exchange'
		MT_ConstantPressurePumpModel,		// Keyword: ConstantPressurePumpModel	'A pump with constant pressure.'
		MT_HeatExchanger,					// Keyword: HeatExchanger				'Simple heat exchanger with given heat flux'
		MT_HeatPump,						// Keyword: HeatPump					'A heat pump.'
		MT_GasBoiler,						// Keyword: GasBoiler					'Gas boiler.'
		MT_ControlValve,					// Keyword: ControlValve				'Control valve.'
		MT_WaterStorage,					// Keyword: WaterStorage				'Water storage.'
		MT_ComponentConditionSystem,		// Keyword: ComponentConditionSystem	'Component conditioning system is a system for heating or cooling of components.'
		MT_Radiator,						// Keyword: Radiator					'Radiator.'
		MT_Mixer,							// Keyword: Mixer						'Mixer component.'
		MT_FMU,								// Keyword: FMU							'Flow characteristics provided by FMU'
		NUM_MT
	};

	/*! Parameters for the component. */
	enum para_t {
		P_PressureLossCoefficient,			// Keyword: PressureLossCoefficient				[-]		'Pressure loss coefficient for the component.'
		P_MaximumPressureLossCoefficient,	// Keyword: MaximumPressureLossCoefficient		[-]		'Maximum pressure loss coefficient for the component.'
		P_HydraulicDiameter,				// Keyword: HydraulicDiameter					[m]		'Inside hydraulic diameter for the component.'
		P_PipeFrictionFactor,				// Keyword: PipeFrictionFactor					[-]		'Pipe friction factor for the component.'
		P_ExternalHeatTransferCoefficient,	// Keyword: ExternalHeatTransferCoefficient		[W/m2K]	'External heat transfer coeffient for the outside boundary.'
		P_TemperatureTolerance,				// Keyword: TemperatureTolerance				[K]		'Temperature tolerance for e.g. thermostats.'
		P_PressureHead,						// Keyword: PressureHead						[Pa]	'Pressure head form a pump.'
		P_PumpEfficiency,					// Keyword: PumpEfficiency						[---]	'Pump efficiency.'
		P_MotorEfficiency,					// Keyword: MotorEfficiency						[---]	'Motor efficiency for a pump.'
		P_RatedHeatingCapacity,				// Keyword: RatedHeatingCapacity				[W]		'Rated heating capacity of the component.'
		P_RatedCoolingCapacity,				// Keyword: RatedCoolingCapacity				[W]		'Rated Cooling capacity of the component.'
		P_AuxiliaryPower,					// Keyword: AuxiliaryPower						[W]		'Auxiliary power of the component.'
		P_Volume,							// Keyword: Volume								[m3]	'Water or air volume of the component.'
		P_ExternalSurfaceArea,				// Keyword: ExternalSurfaceArea					[m2]	'External surface area of the component.'
		P_ConvectiveFraction,				// Keyword: ConvectiveFraction					[---]	'Convective fraction for heating or cooling.'
		P_COP,								// Keyword: COP									[-]		'Coefficient of performance of the component.'
		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID for this component. */
	unsigned int					m_id			= NANDRAD::INVALID_ID;				// XML:A:required

	/*! Display name. */
	std::string						m_displayName;										// XML:A

	/*! Model type. */
	modelType_t						m_modelType		= NUM_MT;							// XML:A:required

	/*! Parameters of the flow component. */
	IBK::Parameter					m_para[NUM_P];										// XML:E

	/* irgendwie muss noch eine verlinkung der component mit anderen objekten hergestellt werden. Z.B. einer anderen Zone
	   oder mit einem Zeitplan der eine Temperatur vorgibt wie sich die Außenrandbedingung darstellt.
	   das sollte am besten ins network element geschoben werden
	*/

	/*	wirkungsgradkurven wie sollten diese abgelegt werden?
		hierbei kann es recht schnell passieren dass diese von einlasstemperaturen und oder massströmen und
		weiteren elemente abhängen.
	*/
};

} // namespace NANDRAD

#endif // NANDRAD_HydraulicNetworkComponentH
