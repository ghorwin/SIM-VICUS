#ifndef NANDRAD_HydraulicNetworkElementH
#define NANDRAD_HydraulicNetworkElementH

#include <IBK_Parameter.h>
#include <IBK_IntPara.h>

#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Constants.h"

namespace NANDRAD {

/*! Contains data of a flow element inside a network.

*/
class HydraulicNetworkElement {
public:

	HydraulicNetworkElement(){}

	HydraulicNetworkElement(unsigned int id, unsigned int inletNodeId, unsigned int outletNodeId,
							unsigned int componentId):
		m_id(id),
		m_inletNodeId(inletNodeId),
		m_outletNodeId(outletNodeId),
		m_componentId(componentId)
	{}

	/*! Parameters for the element . */
	enum para_t {
		P_Length,							// Keyword: Length								[-]		'Length for the element.'
		P_PipeFrictionFactor,				// Keyword: PipeFrictionFactor					[-]		'Pipe friction factor for the element.'
		P_HeatExchangeRate,					// Keyword: HeatExchangeRate					[W]		'Heat flux added or extracted by heat exchanger'
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

	/*! Display name. */
	std::string						m_displayName;										// XML:A

	/*! Parameters of the flow component. */
	IBK::Parameter					m_para[NUM_P];										// XML:E

	/*! Element belongs to this zone (zone id). */
	unsigned int					m_zoneId		= NANDRAD::INVALID_ID;				// XML:E
};

} // namespace NANDRAD

#endif // NANDRAD_HydraulicNetworkElementH
