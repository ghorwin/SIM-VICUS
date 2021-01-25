#ifndef NANDRAD_HydraulicNetworkElementH
#define NANDRAD_HydraulicNetworkElementH

#include <IBK_Parameter.h>
#include <IBK_IntPara.h>
#include <IBK_Path.h>
#include <IBK_LinearSpline.h>

#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Constants.h"

namespace NANDRAD {

class HydraulicNetworkComponent;
class HydraulicNetworkPipeProperties;
class HydraulicNetwork;

/*! Contains data of a flow element inside a network.
	Pipe-specific parameters are stored in HydraulicNetworkPipeProperties objects, referenced via m_pipeID. For
	other elements, this ID is unused.
*/
class HydraulicNetworkElement {
public:

	HydraulicNetworkElement(){}

	/*! C'tor for a network element other than pipes. */
	HydraulicNetworkElement(unsigned int id, unsigned int inletNodeId, unsigned int outletNodeId, unsigned int componentId):
		m_id(id),
		m_inletNodeId(inletNodeId),
		m_outletNodeId(outletNodeId),
		m_componentId(componentId),
		m_pipePropertiesId(INVALID_ID)
	{}

	/*! Specific constructor to create pipe elements. */
	HydraulicNetworkElement(unsigned int id, unsigned int inletNodeId, unsigned int outletNodeId,
							unsigned int componentId, unsigned int pipeID, double length);

	/*! Parameters for the element . */
	enum para_t {
		P_Length,						// Keyword: Length									[m]		'Pipe length'
		P_Temperature,					// Keyword: Temperature								[C]		'Temperature for heat exchange'
		P_HeatFlux,						// Keyword: HeatFlux								[W]		'Heat flux for heat exchange'
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

	/*! Checks for valid and required parameters (value ranges). */
	void checkParameters(const HydraulicNetwork & nw);

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID for this flow component.
		ID is used for outputs and to reference heat sources/sinks connected to this element. For active elements,
		this ID is used to connect control models.
	*/
	unsigned int					m_id				= NANDRAD::INVALID_ID;				// XML:A:required
	/*! Inlet node ID. */
	unsigned int					m_inletNodeId		= NANDRAD::INVALID_ID;				// XML:A:required
	/*! Outlet node ID. */
	unsigned int					m_outletNodeId		= NANDRAD::INVALID_ID;				// XML:A:required
	/*! Hydraulic component ID. */
	unsigned int					m_componentId		= NANDRAD::INVALID_ID;				// XML:A:required
	/*! Pipe ID (only needed for elements that are pipes). */
	unsigned int					m_pipePropertiesId	= NANDRAD::INVALID_ID;				// XML:A

	/*! Display name. */
	std::string						m_displayName;											// XML:A

	/*! Parameters of the flow component. */
	IBK::Parameter					m_para[NUM_P];											// XML:E

	/*! Integer parameters. */
	IBK::IntPara					m_intPara[NUM_IP];										// XML:E

	// *** Variables used only during simulation ***

	const HydraulicNetworkComponent			*m_component				= nullptr;
	const HydraulicNetworkPipeProperties	*m_pipeProperties			= nullptr;

};

} // namespace NANDRAD

#endif // NANDRAD_HydraulicNetworkElementH
