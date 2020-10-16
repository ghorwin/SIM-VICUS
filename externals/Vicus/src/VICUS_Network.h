#ifndef NETWORK_H
#define NETWORK_H

#include <vector>
#include <set>

namespace IBK {
	class Path;
}

namespace VICUS {

class Network {
public:

	class Edge;

	class Node {
	public:

		enum NodeType {
			NT_Building,
			NT_Mixer,
			NT_Source,
			NUM_NT
		};

		Node(const unsigned id, const double &x, const double &y, const NodeType type):
			m_id(id),
			m_x(x),
			m_y(y),
			m_type(type)
		{}

		void collectConnectedEdges(std::set<const Node*> & connectedNodes,
			std::set<const Edge*> & connectedEdge) const;

		unsigned int m_id;
		double m_x, m_y;

		NodeType m_type = NUM_NT;

		std::vector<Edge*>	m_edges;
	};

	class Edge {
	public:

		Edge(const unsigned nodeId1, const unsigned nodeId2, const bool supply):
			m_nodeId1(nodeId1),
			m_nodeId2(nodeId2),
			m_supply(supply)
		{}

		void collectConnectedNodes(std::set<const Node*> & connectedNodes,
			std::set<const Edge*> & connectedEdge) const;

		unsigned int m_nodeId1 = 0;
		unsigned int m_nodeId2 = 0;

		Node *	m_node1 = nullptr;
		Node *	m_node2 = nullptr;

		/*! Diameter in [m] */
		double		m_diameter;

		/*! If false, this is a branch. */
		bool		m_supply;

		/*! Effective length [m], might be different than geometric length between nodes. */
		double		m_length;
	};

	Network();

	unsigned addNode(const double &x, const double &y, const Node::NodeType type, const bool considerCoordinates=false);

	bool addEdge(const unsigned nodeId1, const unsigned nodeId2, const bool supply);

	void readGridFromCSV(const IBK::Path & csv);

	/*! Process all edges vs. all other edges and insert intersection nodes when
		edges intersect between end points.
	*/
	void generateIntersections();

	/*! Should be called whenever m_nodes or m_edges has been modified. */
	void updateNodeEdgeConnectionPointers();

	/*! Checks that all edges and nodes are connected with each other (i.e. single graph network). */
	bool checkConnectedGraph() const;

	void connectBuildings();


	/*! Nodes ID matches always node index.
		\code
		Edge e;
		e.m_n1 = 17;
		// get node identified by edge
		Node & n = m_nodes[e.m_n1];
		\endcode
	*/
	std::vector<Node>		m_nodes;
	std::vector<Edge>		m_edges;
	std::vector<Edge>		m_edgesReduced;
};

} // namespace VICUS


#endif // NETWORK_H
