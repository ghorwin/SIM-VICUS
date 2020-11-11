#ifndef VICUS_NetworkNodeH
#define VICUS_NetworkNodeH

#include <vector>
#include <set>
#include <limits>

#include "VICUS_Constants.h"

namespace VICUS {

class Edge;

class Node {

public:

	enum NodeType {
		NT_Building,
		NT_Mixer,
		NT_Source,
		NUM_NT
	};

	Node();

	Node(const unsigned id, const double &x, const double &y, const NodeType type, const double heatDemand=0):
		m_id(id),
		m_x(x),
		m_y(y),
		m_type(type),
		m_heatingDemand(heatDemand)
	{}

	void collectConnectedEdges(std::set<const Node*> & connectedNodes,
		std::set<const Edge*> & connectedEdge) const;

	/*! updates m_isDeadEnd. If node has less than two neighbours which are not a deadEnd and node is not a building
	 * nor a source: m_isDeadEdnd = true */
	void updateIsDeadEnd();

	/*! Caution: for some applications this definition may needs to be more precise
	 * e.g. compare types of connected edges */
	bool isRedundant() const{
		return (m_edges.size()==2);
	}

	/*! Only callable if node has exactly two edges: return the edge which is not the given edge
		otherwise IBK_ASSERT
	*/
	Edge * neighborEdge(const Edge * e) const;

	/*! get a set of all redundant nodes in the graph */
	void findRedundantNodes(std::set<unsigned> & redundantNodes, std::set<const Edge*> & visitedEdges) const;

	/*! looking from this node in the direction of the given edgeToVisit: return the next node that is not redundant
	 * and the distance to this node */
	const Node * findNextNonRedundantNode(std::set<unsigned> & redundantNodes, double & distance, const Edge* edgeToVisit) const;

	/*! simple algorithm to find the path from this node to the node of type NT_SOURCE.
	 * The path is stored as a set of edges */
	bool findPathToSource(std::set<Edge*> &path, std::set<Edge*> &visitedEdges, std::set<unsigned> &visitedNodes);

	/*! used for dijkstra algorithm. Look at all neighbour nodes: if the m_distanceToStart of this node + the distance to the neighbour
	 * is shorter than the current m_distanceToStart of the neighbour, update it. This makes sure the neighbour nodes have assigned
	 * the currently smallest distance from start */
	void updateNeighbourDistances();

	/*! used for dijkstra algorithm. appends the edge which leads to the predecessor node to path and calls itself for the predecessor node
	 * until a node without predecessor is reached. this way the path from a building to the source can be created, if the predecessors have been set */
	void pathToNull(std::vector<Edge * > & path);

	/*! looks at all adjacent nodes to find a node which has a heating demand >0 and returns it. */
	double adjacentHeatingDemand(std::set<Edge*> visitedEdges);

	unsigned int			m_id  = INVALID_ID;
	double m_x, m_y;//dafür wieder vector oder point nehmen
	NodeType				m_type = NUM_NT;
	double					m_heatingDemand = 0;
	double					m_distanceToStart = std::numeric_limits<double>::max();
	Node *					m_predecessor = nullptr;
	bool					m_isDeadEnd = false;
	std::vector<Edge*>		m_edges;
};

} // namespace VICUS

#endif // NODE_H
