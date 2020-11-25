#ifndef VICUS_NetworkNodeH
#define VICUS_NetworkNodeH

#include "VICUS_Constants.h"
#include "VICUS_CodeGenMacros.h"

#include <vector>
#include <set>
#include <limits>
#include <IBKMK_Vector3D.h>


namespace VICUS {

class NetworkEdge;

class NetworkNode {

public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE

	enum NodeType {
		NT_Building,		// Keyword: Building
		NT_Mixer,			// Keyword: Mixer
		NT_Source,			// Keyword: Source
		NUM_NT
	};

	NetworkNode();

	NetworkNode(const unsigned id, const NodeType type, const IBKMK::Vector3D &v, const double heatDemand=0):
		m_id(id),
		m_position(v),
		m_type(type),
		m_maxHeatingDemand(heatDemand)
	{}

	void collectConnectedEdges(std::set<const NetworkNode*> & connectedNodes,
		std::set<const NetworkEdge*> & connectedEdge) const;

	/*! updates m_isDeadEnd. If node has less than two neighbours which are not a deadEnd and node is not a building
	 * nor a source: m_isDeadEdnd = true */
	void updateIsDeadEnd();

	/*! Caution: for some applications this definition may needs to be more precise
	 * e.g. compare types of connected edges */
	bool isRedundant() const{
		return (m_edges.size()==2);
	}

	/*! Only callable if node has exactly two edges: return the edge which is not the given edge */
	NetworkEdge * neighborEdge(const NetworkEdge * e) const;

	/*! get a set of all redundant nodes in the graph */
	void findRedundantNodes(std::set<unsigned> & redundantNodes, std::set<const NetworkEdge*> & visitedEdges) const;

	/*! looking from this node in the direction of the given edgeToVisit: return the next node that is not redundant
	 * and the distance to this node */
	const NetworkNode * findNextNonRedundantNode(std::set<unsigned> & redundantNodes, double & distance, const NetworkEdge* edgeToVisit) const;

	/*! simple algorithm to find the path from this node to the node of type NT_SOURCE.
	 * The path is stored as a set of edges */
	bool findPathToSource(std::set<NetworkEdge*> &path, std::set<NetworkEdge*> &visitedEdges, std::set<unsigned> &visitedNodes);

	/*! used for dijkstra algorithm. Look at all neighbour nodes: if the m_distanceToStart of this node + the distance to the neighbour
	 * is shorter than the current m_distanceToStart of the neighbour, update it. This makes sure the neighbour nodes have assigned
	 * the currently smallest distance from start */
	void updateNeighbourDistances();

	/*! used for dijkstra algorithm. appends the edge which leads to the predecessor node to path and calls itself for the predecessor node
	 * until a node without predecessor is reached. this way the path from a building to the source can be created, if the predecessors have been set */
	void pathToNull(std::vector<NetworkEdge * > & path);

	/*! looks at all adjacent nodes to find a node which has a heating demand >0 and returns it. */
	double adjacentHeatingDemand(std::set<NetworkEdge*> visitedEdges);


	// *** PUBLIC MEMBER VARIABLES ***

	unsigned int					m_id  = INVALID_ID;										// XML:A:required
	IBKMK::Vector3D					m_position = IBKMK::Vector3D(-9.99,-9.99,-9.99);		// XML:A:required
	NodeType						m_type = NUM_NT;										// XML:A:required
	double							m_maxHeatingDemand = 0;									// XML:A
	double							m_distanceToStart = std::numeric_limits<double>::max();
	NetworkNode *					m_predecessor = nullptr;
	bool							m_isDeadEnd = false;
	std::vector<NetworkEdge*>		m_edges;

};

} // namespace VICUS

#endif // NODE_H
