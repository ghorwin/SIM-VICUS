#ifndef VICUS_NetworkH
#define VICUS_NetworkH

#include "VICUS_NetworkEdge.h"
#include "VICUS_NetworkNode.h"
#include "VICUS_CodeGenMacros.h"
#include "VICUS_NetworkPipe.h"
#include "VICUS_Database.h"

#include <vector>
#include <set>
#include <string>
#include <limits>
#include <string>

#include <IBK_rectangle.h>

#include <NANDRAD_HydraulicNetwork.h>
#include <NANDRAD_HydraulicNetworkComponent.h>

#include "VICUS_Object.h"
#include "VICUS_NetworkController.h"

namespace IBK {
	class Path;
}

namespace VICUS {

class NetworkFluid;

class Network : public Object {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	enum NetworkType {
		NET_SinglePipe,						// Keyword: SinglePipe
		NET_DoublePipe,						// Keyword: DoublePipe
		NUM_NET
	};

	enum para_t{
		P_TemperatureSetpoint,				// Keyword: TemperatureSetpoint					[C]		'Temperature for pipe dimensioning algorithm'
		P_TemperatureDifference,			// Keyword: TemperatureDifference				[K]		'Temperature difference for pipe dimensioning algorithm'
		P_MaxPressureLoss,					// Keyword: MaxPressureLoss						[Pa/m]	'Maximum pressure loss for pipe dimensioning algorithm'
		P_ReferencePressure,				// Keyword: ReferencePressure					[Pa]	'Reference pressure applied to reference element'
		P_DefaultFluidTemperature,			// Keyword: DefaultFluidTemperature				[C]		'Fluid temperature for hydraulic calculation, else initial temperature'
		P_InitialFluidTemperature,			// Keyword: InitialFluidTemperature				[C]		'Initial Fluid temperature for thermo-hydraulic calculation'
		NUM_P
	};


	Network();

	/*! copies all member except nodes and edges */
	Network copyWithBaseParameters();

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
	int nextUnconnectedBuilding() const;

	/*! stores a copy of the current network without "dead end" nodes (and their connecting edges)
	 * "dead end" nodes have only one connecting edge and are not buildings nor sources  */
	void cleanDeadEnds(Network & cleanNetwork, const unsigned maxSteps=50);

	/*! stores a copy of the network without any redundant edges */
	void cleanRedundantEdges(Network & cleanNetwork) const;

	void cleanShortEdges(Network & cleanNetwork, const double &threshold);

	/*! calculate pipe dimensions using a maximum pressure loss per length and fixed temperature difference
	 * the mass flow rate of each pipe will be calculated based on the heatDemand of connected consumer loads (e.g. buildings)
	 */
	void sizePipeDimensions(const NetworkFluid *fluid, std::vector<const NetworkPipe *> & availablePipes);

	void findSourceNodes(std::vector<NetworkNode> &sources) const;

	void writeNetworkCSV(const IBK::Path &file) const;

	void writePathCSV(const IBK::Path &file, const NetworkNode & node, const std::vector<NetworkEdge *> &path) const;

	void writeBuildingsCSV(const IBK::Path &file) const;

	/*! find shortest Path from given startNode (e.g. a building) to Node with type source
	 * using dijkstra-algorithm, implemented according to Wikipedia and return path as vector of edges
	 */
	void dijkstraShortestPathToSource(NetworkNode &startNode, const NetworkNode &endNode, std::vector<NetworkEdge*> &pathEndToStart);

	/*! Recomputes the min/max coordinates of the network and updates m_extends. */
	void updateExtends();

	IBKMK::Vector3D origin() const;

	/*! set m_origin and normalize all position to this origin */
	void setOrigin(const IBKMK::Vector3D &origin);

	/*! return sum of length of all edges */
	double totalLength() const;

	/*! clear nodes, edges */
	void clear(){
		m_edges.clear();
		m_nodes.clear();
	}

	NetworkEdge *edge(unsigned nodeId1, unsigned nodeId2);

	double numberOfBuildings() const;

	/*! sets defauklt values for m_sizingPara. If m_sizingPara[0].empty(), call this function (e.g. to fill GUI)
	 * before calling sizePipeDimensions() */
	void setDefaultSizingParams();

	void updateVisualizationRadius(const VICUS::Database<VICUS::NetworkPipe> & pipeDB) const;

	void setDefaultColors() const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! unique ID of network */
	unsigned int					m_id = INVALID_ID;							// XML:A:required

	/*! fluid id */
	unsigned int					m_fluidID = INVALID_ID;						// XML:A

	/*! network name */
	std::string						m_name;										// XML:A

	/*! Nodes ID matches always node index.
		\code
		Edge e;
		e.m_n1 = 17;
		// get node identified by edge
		Node & n = m_nodes[e.m_n1];
		\endcode
	*/
	std::vector<NetworkNode>				m_nodes;									// XML:E

	/*! vector with edges */
	std::vector<NetworkEdge>				m_edges;									// XML:E

	/*! List of pipes (ids) that may be used in this network. */
	std::vector<unsigned int>				m_availablePipes;							// XML:E

	/*! origin of the network */
	IBKMK::Vector3D							m_origin = IBKMK::Vector3D(0.0, 0.0, 0.0);	// XML:E

	/*! hydraulic sub networks in the Network */
	std::vector<NANDRAD::HydraulicNetwork>	m_hydraulicSubNetworks;						// XML:E

	NetworkType								m_type = NET_DoublePipe;					// XML:E

	/*! Network Parameters (e.g. for pipe sizing algorithm) */
	IBK::Parameter							m_para[NUM_P];								// XML:E

	double									m_scaleNodes = 30;							// XML:E

	double									m_scaleEdges = 30;							// XML:E

	std::vector<VICUS::NetworkController>	m_controllers;								// XML:E


	/*! Stores visibility information for this network.
		Note: keep the next line - this will cause the code generator to create serialization code
			  for the inherited m_visible variable.
	*/
	//:inherited	bool									m_visible = true;							// XML:A

	// *** RUNTIME VARIABLES ***

	/*! Stores the extends of the network.
		Use the function updateExtends() to compute these.
		\code
		m_extends.set(minX, minY, maxX, maxY);
		\endcode
	*/
	IBK::rectangle<double>					m_extends;

private:

	/*! add node to network based on coordinates and type and return the node id.
	 * When considerCoordinates==true and the given coordinates exist already in the network: return the id of this existing node
		ALWAYS use this function if you add nodes with coordinates that where calculated based on already existing coordinates */
	unsigned addNode(const IBKMK::Vector3D &v, const NetworkNode::NodeType type, const bool considerCoordinates=true);

	/*! addNode using Node constructor */
	unsigned addNode(const NetworkNode & node, const bool considerCoordinates=true);

	/*! Calculates Reynolds number of a moving fluid.
	\param v mean fluid flow velocity
	\param kinVis fluid kinematic viscosity
	\param l characteristic length
	THIS IS A COPY FROM NM_PHYSICS
	*/
	static double ReynoldsNumber(const double &v, const double &kinVis, const double &l);

	/*! friction factor according to swamee-jain euqation (approximation of colebrook-white)
	THIS IS A COPY FROM NM_PHYSICS
	*/
	static double FrictionFactorSwamee(const double &reynolds, const double &diameter, const double &roughness);
};

} // namespace VICUS


#endif // VICUS_NetworkH
