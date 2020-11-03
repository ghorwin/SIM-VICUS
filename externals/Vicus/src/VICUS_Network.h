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

		/*! updates m_isDeadEnd. If node has less than two neighbours which are not a deadEnd and node is not a building
		 * nor a source: m_isDeadEdnd = true */
		void checkIsDeadEnd();

		/*! Caution: for some applications this definition may needs to be more precise
		 * e.g. compare types of connected edges */
		bool isRedundant() const{
			return (m_edges.size()==2);
		}

		/*! if node has exactly two edges: return the edge which is not the given edge
		 * otherwise IBK_ASSERT */
		Edge * neighborEdge(const Edge * e) const;

		/*! get a set of all redundant nodes in the graph */
		void findRedundantNodes(std::set<unsigned> & redundantNodes, std::set<const Network::Edge*> & visitedEdges) const;

		/*! looking from this node in the direction of the given edgeToVisit: return the next node that is not redundant
		 * and the distance to this node */
		const Node * findNextNonRedundantNode(std::set<unsigned> & redundantNodes, double & distance, const Network::Edge* edgeToVisit) const;

		/*! simple algorithm to find the path from this node to the node of type NT_SOURCE.
		 * The path is stored as a set of edges */
		bool findPathToSource(std::set<Edge*> &path, std::set<Edge*> &visitedEdges, std::set<unsigned> &visitedNodes);

		/*! used for dijkstra algorithm. Look at all neighbour nodes: if the m_distanceToStart of this node + the distance to the neighbour
		 * is shorter than the current m_distanceToStart of the neighbour, update it. This makes sure the neighbour nodes have assigned
		 * the currently smallest distance from start */
		void updateNeighbourDistances();

		/*! used for dijkstra algorithm. appends the edge which leads to the predecessor node to path and calls itself for the predecessor node
		 * until a node without predecessor is reached. this way the path from a building to the source can be created, if the predecessors have been set */
		void getPathToNull(std::vector<Edge * > & path);

		/*! looks at all adjacent nodes to find a node which has a heating demand >0 and returns it. */
		double adjacentHeatingDemand(std::set<Edge*> visitedEdges);

		unsigned int m_id;
		double m_x, m_y;
		NodeType m_type = NUM_NT;
		double m_heatingDemand = 0;
		double m_distanceToStart = std::numeric_limits<double>::max();
		Node * m_predecessor = nullptr;
		bool isDeadEnd = false;
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

		/*! returns opposite node of the given one */
		Node * neighbourNode(const Network::Node *node) const;

		unsigned int m_nodeId1 = 0;
		unsigned int m_nodeId2 = 0;

		Node		*	m_node1 = nullptr;
		Node		*	m_node2 = nullptr;

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

		/*! determines wether the given point is on the line, between the determining points but does not match any of the determining points */
		bool containsPoint(const double & xp, const double &yp) const;

		/*! determine wether line shares an intersection point wiht given line. The intersection point must be within both lines */
		bool sharesIntersection(const Line &line) const;

		/*! returns length of the line */
		double length() const;

		/*! retruns distance between two given points */
		static double distanceBetweenPoints(const double &x1, const double &y1, const double &x2, const double &y2);

		/*! checks wether the distance between two points is below the threshold */
		static bool pointsMatch(const double &x1, const double &y1, const double &x2, const double &y2, const double threshold=0.01);

		double m_x1;
		double m_y1;
		double m_x2;
		double m_y2;

	};


	Network();

	/*! add node to network based on coordinates and type and return the node id.
	 * When considerCoordinates==true and the given coordinates exist already in the network: return the id of this existing node */
	unsigned addNode(const double &x, const double &y, const Node::NodeType type, const bool considerCoordinates=true);

	/*! addNode using Node constructor */
	unsigned addNode(const Node & node, const bool considerCoordinates=true);

	/*! add Edge based on node ids */
	void addEdge(const unsigned nodeId1, const unsigned nodeId2, const bool supply);

	/*! add Edge using edge constructor */
	void addEdge(const Edge &edge);

	/*! reads csv-files from QGIS with multiple rows, containing "MULTILINESTRING"s and adds according nodes/edges to the network.
		Lines that share a common node (identical coordinates) are automatically connected.
	*/
	void readGridFromCSV(const IBK::Path & filePath);

	/*! reads csv-files from QGIS with multiple rows, containing "POINT"s and adds according nodes of type NT_BUILDING to the network.
	*/
	void readBuildingsFromCSV(const IBK::Path & filePath, const double & heatDemand);

	/*! finds node that is closest to the given coordinates and change its type to NT_SOURCE */
	void setSource(const double &x, const double &y);

	/*! generate all intersections in the network (runs in a loop as long as findAndAddIntersection() is true.) */
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

	/*! returns the first id in m_nodes, which is an unconnected building */
	int nextUnconnectedBuilding();

	/*! stores a copy of the current network without "dead end" nodes (and their connecting edges)
	 * "dead end" nodes have only one connecting edge and are not buildings nor sources  */
	void networkWithoutDeadEnds(Network & cleanNetwork, const unsigned maxSteps);

	/*! calculate the lengths of all edges in the network */
	void calculateLengths();

	void sizePipeDimensions(const double &dpMax, const double &dT, const double &fluidDensity, const double &fluidKinViscosity, const double &roughness);

	/*! stores a copy of the network without any redundant edges */
	void networkWithReducedEdges(Network & reducedNetwork);

	void writeNetworkCSV(const IBK::Path &file) const;

	void writePathCSV(const IBK::Path &file, const VICUS::Network::Node & node, const std::vector<VICUS::Network::Edge*> &path) const;

	void writeBuildingsCSV(const IBK::Path &file) const;

	/*! find shortest Path from given startNode (e.g. a building) to Node with type source
	 * using dijkstra-algorithm, implemented according to Wikipedia and returns path as vector of edges
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
