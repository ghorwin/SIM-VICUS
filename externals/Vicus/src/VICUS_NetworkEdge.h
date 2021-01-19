#ifndef VICUS_NetworkEdgeH
#define VICUS_NetworkEdgeH

#include "VICUS_NetworkNode.h"
#include "VICUS_Constants.h"
#include "VICUS_CodeGenMacros.h"
#include "VICUS_Object.h"
#include "VICUS_NetworkPipe.h"

#include <NANDRAD_HydraulicNetworkComponent.h>

#include <vector>
#include <set>

namespace VICUS {

/*! A network edge defines all parameters needed to generate pipe hydraulic network elements.
	The pipe-specific hydraulic parameters are stored in the pipe database (NetworkPipe).
	The type of pipe model is defined in m_modelType and the kind of heat exchange to be considered
	is defined in m_heatExchangeType.
	Parameters for generation of the hydraulic element are stored in m_para. Also, the double-parameters needed
	for the heat exchange models are stored in m_para.


*/
class NetworkEdge : public Object {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE

	NetworkEdge()
	{}
	NetworkEdge(const unsigned nodeId1, const unsigned nodeId2, const bool supply, const double &length, const unsigned pipeId):
		m_supply(supply),
		m_pipeId(pipeId),
		m_nodeId1(nodeId1),
		m_nodeId2(nodeId2),
		m_length(length)
	{}

	void collectConnectedNodes(std::set<const NetworkNode*> & connectedNodes,
								std::set<const NetworkEdge*> & connectedEdge) const;

	void setInletOutletNode(std::set<const NetworkNode*> & visitedNodes,
					std::set<NetworkEdge*> & orderedEdges);

	bool operator==(const NetworkEdge &e2){
		return (m_nodeId1 == e2.m_nodeId1) && (m_nodeId2 == e2.m_nodeId2);
	}

	bool operator!=(const NetworkEdge &e2){
		return (m_nodeId1 != e2.m_nodeId1) || (m_nodeId2 == e2.m_nodeId2);
	}

	/*! returns opposite node of the given one */
	NetworkNode * neighbourNode(const NetworkNode *node) const;

	unsigned neighbourNode(unsigned nodeId) const;

	double length() const;

	void setLengthFromCoordinates();

	unsigned int nodeId1() const;

	// sets nodeId and pointer to the node and calculates the new length of this edge
	void setNodeId1(unsigned int nodeId1, NetworkNode *node1);

	unsigned int nodeId2() const;

	// sets nodeId and pointer to the node and calculates the new length of this edge
	void setNodeId2(unsigned int nodeId2);

	// *** PUBLIC MEMBER VARIABLES ***

	NetworkNode											*m_node1 = nullptr;
	NetworkNode											*m_node2 = nullptr;

	/*! If false, this is a branch. */
	bool												m_supply;						// XML:A

	/*! ID of pipe in database */
	unsigned int										m_pipeId = INVALID_ID;			// XML:E

	/*! ID of component parameters. */
	unsigned int										m_componentId = INVALID_ID;		// XML:E

	// not final !!! //
	IBK::Parameter										m_ambientTemperature;

	/*! Whether the node is visible or not.
		Note: keep the next line - this will cause the code generator to create serialization code
			  for the inherited m_visible variable.
	*/
	//:inherited	bool												m_visible = true;		// XML:A

	// *** RUNTIME VARIABLES ***

	/*! The radius used for the visualization of this edge in the 3D scene
		Updated whenever the scale factor Network::m_scaleEdges changes, or the pipe ID.
	*/
	double												m_visualizationRadius;

	/*! heating demand of all connected buildings */
	double												m_maxHeatingDemand = 0;

	unsigned int										m_nodeIdInlet = INVALID_ID;
	unsigned int										m_nodeIdOutlet = INVALID_ID;

private:

	// *** PRIVATE MEMBER VARIABLES ***

	unsigned int										m_nodeId1 = 0;					// XML:A:required
	unsigned int										m_nodeId2 = 0;					// XML:A:required

	/*! Effective length [m], might be different than geometric length between nodes. */
	double												m_length = 0;					// XML:E


};


} // namespace VICUS

#endif // EDGE_H
