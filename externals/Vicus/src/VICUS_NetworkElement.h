#ifndef NETWORKELEMENT_H
#define NETWORKELEMENT_H

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_NetworkHeatExchange.h"

namespace VICUS {


/*! This is basically a copy of NANDRAD::HydraulicNetworkElement and used only for VICUS::SubNetwork */

class NetworkElement
{
public:
	NetworkElement();


	/*! C'tor for a network element other than pipes. */
	NetworkElement(unsigned int id, unsigned int inletNodeId, unsigned int outletNodeId, unsigned int componentId):
		m_id(id),
		m_inletNodeId(inletNodeId),
		m_outletNodeId(outletNodeId),
		m_componentId(componentId),
		m_pipePropertiesId(INVALID_ID)
	{
	}

	/*! Specific constructor to create pipe elements. */
	NetworkElement(unsigned int id, unsigned int inletNodeId, unsigned int outletNodeId,
							unsigned int componentId, unsigned int pipeID, double length);

	/*! Parameters for the element . */
	enum para_t {
		P_Length,						// Keyword: Length									[m]		'Pipe length'
		NUM_P
	};

	/*! Whole number parameters. */
	enum intPara_t {
		IP_NumberParallelPipes,			// Keyword: NumberParallelPipes								'Number of parallel pipes'
		NUM_IP
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID


	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID for this flow component.
		ID is used for outputs and to reference heat sources/sinks connected to this element. For active elements,
		this ID is used to connect control models.
	*/
	IDType							m_id				= INVALID_ID;				// XML:A:required
	/*! Inlet node ID. */
	unsigned int					m_inletNodeId		= INVALID_ID;				// XML:A:required
	/*! Outlet node ID. */
	unsigned int					m_outletNodeId		= INVALID_ID;				// XML:A:required
	/*! Hydraulic component ID. */
	unsigned int					m_componentId		= INVALID_ID;				// XML:A:required
	/*! Pipe ID (only needed for elements that are pipes). */
	unsigned int					m_pipePropertiesId	= INVALID_ID;				// XML:A

	/*! Optional reference to a flow controller element. */
	IDType							m_controlElementId = INVALID_ID;				// XML:A

	/*! Display name. */
	std::string						m_displayName;									// XML:A

	/*! Parameters of the flow component. */
	IBK::Parameter					m_para[NUM_P];									// XML:E

	/*! Integer parameters. */
	IBK::IntPara					m_intPara[NUM_IP];								// XML:E

	/*! Optional definition of heat exchange calculation model (if missing, flow element is adiabat). */
	NetworkHeatExchange	m_heatExchange;												// XML:E
};


} // namespace VICUS

#endif // NETWORKELEMENT_H
