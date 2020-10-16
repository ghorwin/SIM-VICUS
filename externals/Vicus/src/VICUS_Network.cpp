#include "VICUS_Network.h"

#include <IBK_assert.h>
#include <IBK_Path.h>


namespace VICUS {

Network::Network() {

}

void Network::readFromCSV(const IBK::Path & csv)
{

}

void Network::generateIntersections()
{

}

void Network::updateNodeEdgeConnectionPointers() {
	// resolve all node and edge pointers

	// first clear edge pointers in all nodes
	for (Node & n : m_nodes)
		n.m_edges.clear();

	const unsigned int nodeCount = m_nodes.size();
	// loop over all edges
	for (Edge & e : m_edges) {
		// store pointers to connected nodes
		IBK_ASSERT(e.m_n1 < nodeCount);
		e.m_node1 = &m_nodes[e.m_n1];
		IBK_ASSERT(e.m_n2 < nodeCount);
		e.m_node2 = &m_nodes[e.m_n2];

		// now also store pointer to this edge into connected nodes
		e.m_node1->m_edges.push_back(&e);
		e.m_node2->m_edges.push_back(&e);
	}

	// traverse tree starting from node 1
	// node1->m_edges[0]->m_node1->m_edges[1]
	// for (Edge * e : n->m_edges) {
	//   e->m_node1->traverse();
	//   e->m_node2->traverse();
	//}
}


bool Network::checkConnectedGraph() const {
	std::set<const Node*> connectedNodes;
	std::set<const Edge*> connectedEdge;

	// start by any node
	const Edge * start = &m_edges[0];

	// ask edge to check its nodes
	start->collectConnectedNodes(connectedNodes, connectedEdge);


	return (connectedEdge.size() && m_edges.size() && connectedNodes.size() == m_nodes.size());
}


void Network::connectBuildings() {

	// new branch-edges are stored in separate vector and added to global edge vector at end of algorithm

	// modified edges (i.e. when node intersection point is within the existing edge), lead to
	// direct modification of m_edges vector.

	// process all nodes of type building

		// process all edges and compute closest point
			// if point distance is closer than previous find, store it

		// check if intersection point is inside edge -> add node and new edge
		// else: connect to nearest edge-node (left or right)
}


void Network::Edge::collectConnectedNodes(std::set<const Network::Node *> & connectedNodes,
										  std::set<const Network::Edge *> & connectedEdge) const
{
	// first store ourselves as connected
	connectedEdge.insert(this);
	// now ask our nodes to collect their connected elements
	m_node1->collectConnectedEdges(connectedNodes, connectedEdge);
	m_node2->collectConnectedEdges(connectedNodes, connectedEdge);
}


void Network::Node::collectConnectedEdges(std::set<const Network::Node *> & connectedNodes, std::set<const Network::Edge *> & connectedEdge) const {
	// store ourselves as connected
	connectedNodes.insert(this);
	// now ask connected elements to collect their nodes
	for (const Edge * e : m_edges) {
		// only process edges that are not yet collected
		if (connectedEdge.find(e) == connectedEdge.end())
			e->collectConnectedNodes(connectedNodes, connectedEdge);
	}
}




} // namespace VICUS
