#include "VICUS_NetworkEdge.h"

#include <IBK_assert.h>

namespace VICUS {


void Edge::collectConnectedNodes(std::set<const Node *> & connectedNodes,
								 std::set<const Edge *> & connectedEdge) const {
	// first store ourselves as connected
	connectedEdge.insert(this);
	// now ask our nodes to collect their connected elements
	m_node1->collectConnectedEdges(connectedNodes, connectedEdge);
	m_node2->collectConnectedEdges(connectedNodes, connectedEdge);
}


Node * Edge::neighbourNode(const Node *node) const{
	IBK_ASSERT(node->m_id == m_nodeId1 || node->m_id == m_nodeId2);
	if (node->m_id == m_nodeId1)
		return m_node2;
	else
		return m_node1;
}

} // namspace VICUS
