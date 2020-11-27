#ifndef VICUS_NetworkH
#define VICUS_NetworkH

#include "VICUS_NetworkEdge.h"
#include "VICUS_NetworkNode.h"
#include "VICUS_CodeGenMacros.h"

#include <vector>
#include <set>
#include <string>
#include <limits>
#include <string>

#include "IBK_rectangle.h"

namespace IBK {
	class Path;
}

namespace VICUS {

class NetworkPipe;
class NetworkFluid;

class Network {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	Network();

	/*! call private addNode and set position relative to orign.
	 * ALWAYS use this funtion If you add nodes in original coordinates to a network where m_origin may has been already set */
	unsigned addNodeExt(const IBKMK::Vector3D &v, const NetworkNode::NodeType type, const bool considerCoordinates=true){
		return addNode(v - m_origin, type, considerCoordinates);
	}

	/*! add Edge based on node ids */
	void addEdge(const unsigned nodeId1, const unsigned nodeId2, const bool supply);

	/*! add Edge using edge constructor */
	void addEdge(const NetworkEdge &edge);

	/*! reads csv-files from QGIS with multiple rows, containing "MULTILINESTRING"s and adds according nodes/edges to the network.
		Lines that share a common node (identical coordinates) are automatically connected.
	*/
	void readGridFromCSV(const IBK::Path & filePath);

	/*! reads csv-files from QGIS with multiple rows, containing "POINT"s and adds according nodes of type NT_BUILDING to the network.
	*/
	void readBuildingsFromCSV(const IBK::Path & filePath, const double & heatDemand);

	/*! finds node that is closest to the given coordinates and change its type to NT_SOURCE */
	void assignSourceNode(const IBKMK::Vector3D &v);

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

	/*! calculate pipe dimensions using a maximum pressure loss per length and fixed temperature difference
	 * the mass flow rate of each pipe will be calculated based on the heatDemand of connected consumer loads (e.g. buildings)
	 \param deltaPMax maximum pressure loss per length [Pa/m]
	 \param deltaTemp temperature difference between supply and return [K]
	 \param temp temperature used for determination of fluid properties [C]
	 */
	void sizePipeDimensions(const double &deltaPMax, const double &deltaTemp, const double &temp,
							const NetworkFluid &fluid, const std::vector<NetworkPipe> &pipeDB);

	/*! stores a copy of the network without any redundant edges */
	void networkWithReducedEdges(Network & reducedNetwork);

	void writeNetworkCSV(const IBK::Path &file) const;

	void writePathCSV(const IBK::Path &file, const NetworkNode & node, const std::vector<NetworkEdge *> &path) const;

	void writeBuildingsCSV(const IBK::Path &file) const;

	/*! find shortest Path from given startNode (e.g. a building) to Node with type source
	 * using dijkstra-algorithm, implemented according to Wikipedia and return path as vector of edges
	 */
	void dijkstraShortestPathToSource(NetworkNode &startNode, std::vector<NetworkEdge*> &pathToSource);

	/*! Recomputes the min/max coordinates of the network and updates m_extends. */
	void updateExtends();

	/*! pressure loss of a rough pipe according to colebrook equation
	 \param length in [m]
	 \param massFlow in [kg/s]
	 \param temperature in [C]
	 */
	static double pressureLossColebrook(const double &length, const double &massFlow, const NetworkFluid &fluid,
										const NetworkPipe &pipe, const double &temperature);

	IBKMK::Vector3D origin() const;

	/*! set m_origin and normalize all position to this origin */
	void setOrigin(const IBKMK::Vector3D &origin);

	/*! return sum of length of all edges */
	double totalLength();

	/*! clear nodes, edges */
	void clear(){
		m_edges.clear();
		m_nodes.clear();
	}


	// *** PUBLIC MEMBER VARIABLES ***

	/*! unique ID of network */
	unsigned int					m_id = INVALID_ID;			// XML:A:required

	/*! fluid id */
	unsigned int					m_fluidID = INVALID_ID;		// XML:A:required

	/*! network name */
	std::string						m_name;						// XML:A

	/*! Nodes ID matches always node index.
		\code
		Edge e;
		e.m_n1 = 17;
		// get node identified by edge
		Node & n = m_nodes[e.m_n1];
		\endcode
	*/
	std::vector<NetworkNode>		m_nodes;									// XML:E:required

	/*! vector with edges */
	std::vector<NetworkEdge>		m_edges;									// XML:E:required

	/*! Stores the extends of the network.
		Use the function updateExtends() to compute these.
		\code
		m_extends.set(minX, minY, maxX, maxY);
		\endcode
	*/
	IBK::rectangle<double>			m_extends;

	/*! origin of the network */
	IBKMK::Vector3D					m_origin = IBKMK::Vector3D(0.0, 0.0, 0.0);	// XML:E:required


private:

	/*! add node to network based on coordinates and type and return the node id.
	 * When considerCoordinates==true and the given coordinates exist already in the network: return the id of this existing node
		ALWAYS use this function if you add nodes with coordinates that where calculated based on already existing coordinates */
	unsigned addNode(const IBKMK::Vector3D &v, const NetworkNode::NodeType type, const bool considerCoordinates=true);

	/*! addNode using Node constructor */
	unsigned addNode(const NetworkNode & node, const bool considerCoordinates=true);

};

} // namespace VICUS


#endif // VICUS_NetworkH
