#include "VICUS_Network.h"

#include <IBK_assert.h>
#include <IBK_Path.h>

#include <fstream>
#include <algorithm>

namespace VICUS {

Network::Network() {

}


unsigned Network::addNode(const double &x, const double &y, const Network::Node::NodeType type, const bool consistentCoordinates)
{
	// if there is an existing node with identical coordinates, return its id and dont add a new one
	if (consistentCoordinates){
		for (Node n: m_nodes){
			if (n.m_x==x && n.m_y==y)
				return n.m_id;
		}
	}
	unsigned id = m_nodes.size();
	m_nodes.push_back(Node(id, x, y, type));
	updateNodeEdgeConnectionPointers();

	return id;
}


bool Network::addEdge(const unsigned nodeId1, const unsigned nodeId2, const bool supply)
{
	if (nodeId1>m_nodes.size()-1 || nodeId2>m_nodes.size()-1)
		return false;
	m_edges.push_back(Edge(nodeId1, nodeId2, supply));
	updateNodeEdgeConnectionPointers();

	return true;
}


void Network::readGridFromCSV(const IBK::Path &csv){
FUNCID(Network::readGridFromCSV);

	// read file
	if (!csv.exists())
		throw IBK::Exception(IBK::FormatString("File '%1' doesn't exist.").arg(csv), FUNC_ID);
	std::ifstream file(csv.wstr());
	std::string line;
	std::vector<std::string> cont;
	while (std::getline(file, line))
		cont.push_back(line);
	file.close();

	// extract vector of string-xy-pairs from ' "MULTILINESTRING ((3 4,1 6,10 2))", '
	// probably there is a more convient way?
	std::vector<std::string> tokens;
	for (std::string line: cont){
		if (line.find("MULTILINESTRING") == std::string::npos)
			continue;
		IBK::trim(line, ",");
		IBK::trim(line, "\"");
		IBK::trim(line, "MULTILINESTRING ((");
		IBK::trim(line, "))");
		IBK::explode(line, tokens, ",", IBK::EF_NoFlags);

		// convert this vector to double and add it as a graph
		std::vector<std::vector<double> > polyLine;
		for (std::string str: tokens){
			std::vector<std::string> xyStr;
			IBK::explode(str, xyStr, " ", IBK::EF_NoFlags);
			polyLine.push_back(std::vector<double> {IBK::string2val<double>(xyStr[0]), IBK::string2val<double>(xyStr[1])});
		}
		for (unsigned i=0; i<polyLine.size()-1; ++i){
			unsigned n1 = addNode(polyLine[i][0], polyLine[i][1], Node::NT_Mixer, true);
			unsigned n2 = addNode(polyLine[i+1][0], polyLine[i+1][1], Node::NT_Mixer, true);
			addEdge(n1, n2, true);
		}
	}



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
		IBK_ASSERT(e.m_nodeId1 < nodeCount);
		e.m_node1 = &m_nodes[e.m_nodeId1];
		IBK_ASSERT(e.m_nodeId2 < nodeCount);
		e.m_node2 = &m_nodes[e.m_nodeId2];

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
										  std::set<const Network::Edge *> & connectedEdge) const {
	// first store ourselves as connected
	connectedEdge.insert(this);
	// now ask our nodes to collect their connected elements
	m_node1->collectConnectedEdges(connectedNodes, connectedEdge);
	m_node2->collectConnectedEdges(connectedNodes, connectedEdge);
}


void Network::Node::collectConnectedEdges(std::set<const Network::Node *> & connectedNodes,
										  std::set<const Network::Edge *> & connectedEdge) const {
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
