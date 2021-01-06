#ifndef NANDRAD_HydraulicNetworkPipePropertiesH
#define NANDRAD_HydraulicNetworkPipePropertiesH

#include <IBK_Parameter.h>

#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Constants.h"

namespace NANDRAD {

/*! Stores data specific for a pipe. */
class HydraulicNetworkPipeProperties {
public:

	/*! Parameters for the component. */
	enum para_t {
		P_PipeRoughness,					// Keyword: PipeRoughness						[mm]	'Roughness of pipe material.'
		P_HydraulicDiameter,				// Keyword: HydraulicDiameter					[mm]	'Inside hydraulic diameter for the pipe.'
		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE
	NANDRAD_COMPARE_WITH_ID
	NANDRAD_COMP(HydraulicNetworkPipeProperties)

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID for this component. */
	unsigned int					m_id			= NANDRAD::INVALID_ID;				// XML:A:required

	/*! Parameters. */
	IBK::Parameter					m_para[NUM_P];										// XML:E

};

} // namespace NANDRAD

#endif // NANDRAD_HydraulicNetworkPipePropertiesH
