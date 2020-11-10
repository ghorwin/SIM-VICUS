#ifndef EDGE_H
#define EDGE_H

#include "VICUS_NetworkNode.h"
#include "VICUS_Constants.h"

#include <vector>
#include <set>

namespace VICUS {

class Edge {

public:

	Edge();
	Edge(const unsigned nodeId1, const unsigned nodeId2, const bool supply):
		m_nodeId1(nodeId1),
		m_nodeId2(nodeId2),
		m_supply(supply)
	{}
	Edge(const unsigned nodeId1, const unsigned nodeId2, const double &length, const double &diameter, const bool supply):
		m_nodeId1(nodeId1),
		m_nodeId2(nodeId2),
		m_length(length),
		m_diameterInside(diameter),
		m_supply(supply)
	{}

	void collectConnectedNodes(std::set<const Node*> & connectedNodes,
							   std::set<const Edge*> & connectedEdge) const;

	bool operator==(const Edge &e2){
		return (m_nodeId1 == e2.m_nodeId1) && (m_nodeId2 == e2.m_nodeId2);
	}

	/*! returns opposite node of the given one */
	Node * neighbourNode(const Node *node) const;

	unsigned int m_nodeId1 = 0;
	unsigned int m_nodeId2 = 0;

	Node		*	m_node1 = nullptr;
	Node		*	m_node2 = nullptr;

	/*! Effective length [m], might be different than geometric length between nodes. */
	double			m_length;

	/*! Inner Diameter in [m] */
	double			m_diameterInside;

	/*! Outer Diameter in [m] */
	double			m_diameterOutside;

	/*! If false, this is a branch. */
	bool			m_supply;

	/*! heating demand of all connected buildings */
	double			m_heatingDemand = 0;
};


} // namespace VICUS

#endif // EDGE_H
