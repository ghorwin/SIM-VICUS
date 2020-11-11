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
		MT_AdiabaticPipe,					// Keyword: AdiabaticPipe				'Pipe without heat exchange'
		MT_UniformTemperaturePipe,			// Keyword: UniformTemperaturePipe		'Pipe with single temperature and heat exchange with surrounding'
		MT_TemperatureDistributionPipe,		// Keyword: TemperatureDistributionPipe	'Pipe with temperature distribution (spatial discretization) and heat exchange with surrounding'
		MT_Pump,							// Keyword: Pump						'A pump with some control regime'
		MT_FMU,								// Keyword: FMU							'Flow characteristics provided by FMU'
		NUM_MT
	};

	/*! Parameters for the pump model. */
	enum para_t {
		P_PressureLossCoefficient,
		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID for this flow component.
		ID is used for outputs and to reference heat sources/sinks connected to this element. For active elements,
		this ID is used to connect control models.
	*/
	unsigned int					m_id			= NANDRAD::INVALID_ID;				// XML:A:required
	/*! Inlet node ID. */
	unsigned int					m_inletNodeId	= NANDRAD::INVALID_ID;				// XML:A:required
	/*! Outlet node ID. */
	unsigned int					m_outletNodeId	= NANDRAD::INVALID_ID;				// XML:A:required
	/*! Hydraulic component ID. */
	unsigned int					m_componentId	= NANDRAD::INVALID_ID;				// XML:A:required

	std::string						m_displayName;										// XML:A

	/*! Model type. */
	modelType_t						m_modelType		= NUM_MT;							// XML:A:required

	/*! Parameters of the flow component. */
	IBK::Parameter					m_para[NUM_P];										// XML:E
};

} // namespace NANDRAD

#endif // NANDRAD_HydraulicNetworkComponentH
