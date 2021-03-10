#ifndef NANDRAD_HYDRAULICNETWORK_H
#define NANDRAD_HYDRAULICNETWORK_H

#include <vector>

#include <IBK_Parameter.h>

#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_HydraulicNetworkElement.h"
#include "NANDRAD_HydraulicFluid.h"
#include "NANDRAD_HydraulicNetworkPipeProperties.h"
#include "NANDRAD_HydraulicNetworkComponent.h"

namespace NANDRAD {

/*! Contains all data for a hydraulic network. */
class HydraulicNetwork {
	NANDRAD_READWRITE_PRIVATE
public:

	/*! The various types (equations) of the hydraulic component. */
	enum ModelType {
		MT_HydraulicNetwork,				// Keyword: HydraulicNetwork				'Only Hydraulic calculation with constant temperature'
		MT_ThermalHydraulicNetwork,			// Keyword: ThermalHydraulicNetwork			'Thermo-hydraulic calculation'
		NUM_MT
	};

	/*! Parameters for the element . */
	enum para_t {
		P_DefaultFluidTemperature,			// Keyword: DefaultFluidTemperature	[C]		'Default temperature for HydraulicNetwork models'
		P_InitialFluidTemperature,			// Keyword: InitialFluidTemperature	[C]		'Initial temperature of the fluid'
		P_ReferencePressure,				// Keyword: ReferencePressure [Pa]			'Reference pressure of network'
		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE_IFNOT_INVALID_ID
	NANDRAD_COMPARE_WITH_ID

	/*! Checks for valid and required parameters (value ranges). */
	void checkParameters( const std::map<std::string, IBK::Path> &placeholders,
						  const std::vector<Zone> &zones,
						  const std::vector<ConstructionInstance> &conInstances) ;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID for this network. */
	unsigned int									m_id			= INVALID_ID;					// XML:A:required
	/*! Descriptive name. */
	std::string										m_displayName;									// XML:A

	ModelType										m_modelType		= MT_ThermalHydraulicNetwork;	// XML:A:required

	/*! at the inlet node of the reference element,
	 * the reference pressure will be applied (usually should be the pump) */
	unsigned int									m_referenceElementId = INVALID_ID;				// XML:A:required

	HydraulicFluid									m_fluid;										// XML:E

	/*! Global network parameters. */
	IBK::Parameter									m_para[NUM_P];									// XML:E

	/*! Pipes used in this network. */
	std::vector<HydraulicNetworkPipeProperties>		m_pipeProperties;								// XML:E
	/*! Hydraulic components used in this network. */
	std::vector<HydraulicNetworkComponent>			m_components;									// XML:E

	/*! List of flow elements that make up this network. */
	std::vector<HydraulicNetworkElement>			m_elements;										// XML:E

};

} // namespace NANDRAD

#endif // NANDRAD_HYDRAULICNETWORK_H
