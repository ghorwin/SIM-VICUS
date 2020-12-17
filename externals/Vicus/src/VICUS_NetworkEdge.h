#ifndef VICUS_NetworkEdgeH
#define VICUS_NetworkEdgeH

#include "VICUS_NetworkNode.h"
#include "VICUS_Constants.h"
#include "VICUS_CodeGenMacros.h"
#include "VICUS_Object.h"

#include <NANDRAD_HydraulicNetworkComponent.h>

#include <vector>
#include <set>

namespace VICUS {

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

	void orderEdges(std::set<const NetworkNode*> & visitedNodes,
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

	/*! id of pipe in database */
	unsigned int										m_pipeId = INVALID_ID;			// XML:E


	NANDRAD::HydraulicNetworkComponent::modelType_t		m_modelType = NANDRAD::HydraulicNetworkComponent::NUM_MT;

	// *** RUNTIME VARIABLES ***

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
