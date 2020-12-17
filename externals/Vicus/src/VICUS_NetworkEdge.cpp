#include "VICUS_NetworkEdge.h"
#include "VICUS_NetworkLine.h"

#include <IBK_assert.h>

namespace VICUS {


void NetworkEdge::collectConnectedNodes(std::set<const NetworkNode *> & connectedNodes,
								 std::set<const NetworkEdge *> & connectedEdge) const {
	// first store ourselves as connected
	connectedEdge.insert(this);
	// now ask our nodes to collect their connected elements
	m_node1->collectConnectedEdges(connectedNodes, connectedEdge);
	m_node2->collectConnectedEdges(connectedNodes, connectedEdge);
}

void NetworkEdge::orderEdges(std::set<const NetworkNode *> &visitedNodes, std::set<NetworkEdge *> &orderedEdges)
{
	// first store ourselves as connected
	orderedEdges.insert(this);
	// now ask our nodes to collect their connected elements
	m_node1->orderEdges(visitedNodes, orderedEdges);
	m_node2->orderEdges(visitedNodes, orderedEdges);
}


unsigned NetworkEdge::neighbourNode(unsigned nodeId) const{
	IBK_ASSERT(nodeId == m_nodeId1 || nodeId == m_nodeId2);
	if (nodeId == m_nodeId1)
		return m_nodeId2;
	else
		return m_nodeId1;
}


NetworkNode * NetworkEdge::neighbourNode(const NetworkNode *node) const{
	IBK_ASSERT(node->m_id == m_nodeId1 || node->m_id == m_nodeId2);
	if (node->m_id == m_nodeId1)
		return m_node2;
	else
		return m_node1;
}

void NetworkEdge::setLengthFromCoordinates(){
	m_length = NetworkLine2D(*this).length();
}

unsigned int NetworkEdge::nodeId1() const
{
	return m_nodeId1;
}


void NetworkEdge::setNodeId1(unsigned int nodeId1, NetworkNode *node1)
{
	m_nodeId1 = nodeId1;
	m_node1 = node1;  // set pointer, so that setLengthFromCoordinates works
	setLengthFromCoordinates();
}

unsigned int NetworkEdge::nodeId2() const
{
	return m_nodeId2;
}

void NetworkEdge::setNodeId2(unsigned int nodeId2)
{
	m_nodeId2 = nodeId2;
	setLengthFromCoordinates();
}

double NetworkEdge::length() const{
	return m_length;
}

} // namspace VICUS
