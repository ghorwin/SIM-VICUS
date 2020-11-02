#ifndef NETWORK_H
#define NETWORK_H

#include <vector>
#include <set>
#include <string>
#include <limits>

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

		bool isDeadEnd() const{
			return (m_edges.size()<2 && m_type != NT_Building && m_type != NT_Source);
		}

		/*! Caution: for some applications this definition may needs to be more precise
		 * e.g. compare types of connected edges */
		bool isRedundant() const{
			return (m_edges.size()==2);
		}

		Edge *neighborEdge(const Edge * e) const;

		void findRedundantNodes(std::set<unsigned> & redundantNodes, std::set<const Network::Edge*> & visitedEdges) const;

		const Node * findNextNonRedundantNode(std::set<unsigned> & redundantNodes, double & totalLength, const Network::Edge* edge) const;

		bool findPathToSource(std::set<Edge*> &path, std::set<Edge*> &visitedEdges, std::set<unsigned> &visitedNodes);

		/*! used for dijkstra algorithm */
		void updateNeighbourDistances();

		void getPathToNull(std::vector<Edge * > & path);

		double adjacentHeatingDemand(std::set<Edge*> visitedEdges);

		unsigned int m_id;
		double m_x, m_y;
		NodeType m_type = NUM_NT;
		double m_heatingDemand = 0;
		double m_distanceToStart = std::numeric_limits<double>::max();
		Node * m_predecessor = nullptr;
		std::vector<Edge*>	m_edges;
	};


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
			m_diameter(diameter),
			m_supply(supply)
		{}

		void collectConnectedNodes(std::set<const Node*> & connectedNodes,
			std::set<const Edge*> & connectedEdge) const;

		bool operator==(const Edge &e2){
			return (m_nodeId1 == e2.m_nodeId1) && (m_nodeId2 == e2.m_nodeId2);
		}

		Node * neighbourNode(const Network::Node *node) const;

		unsigned int m_nodeId1 = 0;
		unsigned int m_nodeId2 = 0;

		Node *	m_node1 = nullptr;
		Node *	m_node2 = nullptr;

		/*! Effective length [m], might be different than geometric length between nodes. */
		double		m_length;

		/*! Diameter in [m] */
		double		m_diameter;

		/*! If false, this is a branch. */
		bool		m_supply;

		/*! heating demand of all connected buildings */
		double		m_heatingDemand = 0;
	};


	class Line{
	public:

		Line(const double &x1, const double &y1, const double &x2, const double &y2):
			m_x1(x1),
			m_y1(y1),
			m_x2(x2),
			m_y2(y2)
		{}

		Line(const Edge &e):
			m_x1(e.m_node1->m_x),
			m_y1(e.m_node1->m_y),
			m_x2(e.m_node2->m_x),
			m_y2(e.m_node2->m_y)
		{}

		/*! return intersection point between two lines */
		void intersection(const Line &line, double &xs, double &ys) const;

		/*! return othogonal projection of point on line */
		void projectionFromPoint(const double &xp, const double &yp, double &xproj, double &yproj) const;

		/*! return orthogonal distance between point and line */
		double distanceToPoint(const double &xp, const double &yp) const;

		/*! returns m, n of linear equation */
		std::pair<double, double>  linearEquation() const;

		/*! determines wether the given point is on the line, between the points that determine this line */
		bool containsPoint(const double & xp, const double &yp) const;

		/*! determine wether line shares an intersection point wiht given line. The intersection point must be within both lines */
		bool sharesIntersection(const Line &line) const;

		double length() const;

		static double distanceBetweenPoints(const double &x1, const double &y1, const double &x2, const double &y2);

		static bool pointsMatch(const double &x1, const double &y1, const double &x2, const double &y2, const double threshold=0.01);

		double m_x1;
		double m_y1;
		double m_x2;
		double m_y2;

	};


	Network();

	unsigned addNode(const double &x, const double &y, const Node::NodeType type, const bool considerCoordinates=true);

	unsigned addNode(const Node & node, const bool considerCoordinates=true);

	void addEdge(const unsigned nodeId1, const unsigned nodeId2, const bool supply);

	void addEdge(const Edge &edge);

	/*! reads csv-files from QGIS with multiple rows, containing "MULTILINESTRING"s and adds according nodes/edges to the network.
		Lines that share a common node (identical coordinates) are automatically connected.
	*/
	void readGridFromCSV(const IBK::Path & filePath);

	/*! reads csv-files from QGIS with multiple rows, containing "POINT"s and adds according nodes of type NT_BUILDING to the network.
	*/
	void readBuildingsFromCSV(const IBK::Path & filePath, const double & heatDemand);

	void setSource(const double &x, const double &y);

	/*! runs as long as findIntersection() is true. If an intersection was found, the according nodes is inserted
	 * and the according edges are manipulated.
	*/
	void generateIntersections();

	/*! Process all edges vs. all other edges. If an intersection was found, set the according
	 * edges and the intersection point are set and return true. If there are no intersection points, return false.
	*/
	bool findAndAddIntersection();

	/*! Should be called whenever m_nodes or m_edges has been modified. */
	void updateNodeEdgeConnectionPointers();

	/*! Checks that all edges and nodes are connected with each other (i.e. single graph network). */
	bool checkConnectedGraph() const;

	/*! iterates through all building nodes, finds closest supply edge and connects the building node to the network */
	void connectBuildings(const bool extendSupplyPipes);

	int nextUnconnectedBuilding();

	/*! cleanNetwork is a copy of the current network without "dead end" nodes (and their connecting edges)
	 * "dead end" nodes have only one connecting edge and are not buildings  */
	void networkWithoutDeadEnds(Network & cleanNetwork) const;

	void calculateLengths();

	void sizePipeDimensions(const double &dpMax, const double &dT, const double &fluidDensity, const double &fluidKinViscosity, const double &roughness);

	void networkWithReducedEdges(Network & reducedNetwork);

	void writeNetworkCSV(const IBK::Path &file) const;

	void writePathCSV(const IBK::Path &file, const VICUS::Network::Node & node, const std::vector<VICUS::Network::Edge*> &path) const;

	void writeBuildingsCSV(const IBK::Path &file) const;

	/*! find shortest Path from given startNode (e.g. a building) to Node with type source
	 * using dijkstra-algorithm, implemented according to Wikipedia
	 * and returns path as vector of edges
	 */
	void dijkstraShortestPathToSource(Node &startNode, std::vector<Edge*> &pathToSource);

	/*! pressure loss of a rough pipe according to colebrook equation */
	static double pressureLossColebrook(const double &diameter, const double &length, const double &roughness, const double &massFlow,
										const double &fluidDensity, const double &fluidKinViscosity);

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


private:

	static void readCSV(const IBK::Path & filePath, std::vector<std::string> & content);

};

} // namespace VICUS


#endif // NETWORK_H
