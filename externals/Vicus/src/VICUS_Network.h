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
		enum UsageType {
			NT_Building,
			NT_Mixer,
			NT_Source,
			NUM_NT
		};

		void collectConnectedEdges(std::set<const Node*> & connectedNodes,
			std::set<const Edge*> & connectedEdge) const;

		unsigned int m_id;
		double m_x, m_y;

		UsageType m_type = NUM_NT;

		std::vector<Edge*>	m_edges;
	};

	class Edge {
	public:

		void collectConnectedNodes(std::set<const Node*> & connectedNodes,
			std::set<const Edge*> & connectedEdge) const;

		unsigned int m_id;
		unsigned int m_n1 = 0;
		unsigned int m_n2 = 0;

		Node *	m_node1 = nullptr;
		Node *	m_node2 = nullptr;

		/*! Diameter in [mm] */
		double		m_d;

		/*! If false, this is a branch. */
		bool		m_supply;

		/*! Effective length [m], might be different than geometric length between nodes. */
		double		m_length;
	};

	Network();

	void readFromCSV(const IBK::Path & csv);

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
