#ifndef NANDRAD_HydraulicNetworkElementH
#define NANDRAD_HydraulicNetworkElementH

#include <IBK_Parameter.h>
#include <IBK_IntPara.h>
#include <IBK_Path.h>

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
		P_Length,							// Keyword: Length								[m]		'Pipe length'
		P_HeatFlux,							// Keyword: HeatFlux							[W]		'Constant heat flux'
		NUM_P
	};

	/*! Integer/whole number parameters. */
	enum intPara_t {
		IP_ZoneId,							// Keyword: ZoneId								[-]		'ID of coupled zone for thermal exchange'
		NUM_IP
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE
	NANDRAD_COMPARE_WITH_ID

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


	/// TODO : Fixme - this wouldn't work in Solver context!
	IBK::Path						m_filepathData;										// XML:E
	IBK::Path						m_filepathFMU;										// XML:E


	/*! Integer parameters. */
	IBK::IntPara					m_intPara[NUM_IP];									// XML:E
};

} // namespace NANDRAD

#endif // NANDRAD_HydraulicNetworkElementH
