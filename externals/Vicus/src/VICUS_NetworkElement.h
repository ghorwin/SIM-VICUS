#ifndef VICUS_NetworkElementH
#define VICUS_NetworkElementH

#include <QString>

#include <IBK_IntPara.h>

#include "VICUS_Constants.h"
#include "VICUS_CodeGenMacros.h"

namespace VICUS {


/*! Data model object for network components, similar as NANDRAD::HydraulicNetworkElement with
	data members needed in the user interface.

	Note: In contrast to NANDRAD::HydraulicNetworkElement,
	we dont store a pipePropertiesId here, this is done in VICUS::NetworkComponent
*/

class NetworkElement {
public:

	NetworkElement() = default;

	/*! C'tor for a network element other than pipes. */
	NetworkElement(unsigned int id, unsigned int inletNodeId, unsigned int outletNodeId, unsigned int componentId):
		m_id(id),
		m_inletNodeId(inletNodeId),
		m_outletNodeId(outletNodeId),
		m_componentId(componentId)
	{
	}


	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID
	VICUS_COMP(NetworkElement)

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID for this flow component.
		ID is used for outputs and to reference heat sources/sinks connected to this element. For active elements,
		this ID is used to connect control models.
	*/
	IDType							m_id				= VICUS::INVALID_ID;				// XML:A:required
	/*! Inlet node ID. */
	IDType							m_inletNodeId		= VICUS::INVALID_ID;				// XML:A:required
	/*! Outlet node ID. */
	IDType							m_outletNodeId		= VICUS::INVALID_ID;				// XML:A:required
	/*! Network component ID. */
	IDType							m_componentId		= VICUS::INVALID_ID;				// XML:A

	/*! Optional reference to a flow controller element. */
	IDType							m_controlElementId	= VICUS::INVALID_ID;				// XML:A

	/*! Display name. */
	QString							m_displayName;											// XML:A
};


}

#endif // NETWORKELEMENT_H
