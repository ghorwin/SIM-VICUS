#include "VICUS_Network.h"
#include "VICUS_NetworkLine.h"
#include "VICUS_NetworkFluid.h"
#include "VICUS_NetworkPipe.h"
#include "VICUS_Project.h"
#include "VICUS_KeywordList.h"

#include <IBK_assert.h>
#include <IBK_Path.h>
#include <IBK_FileReader.h>

#include <fstream>
#include <algorithm>

#include <NANDRAD_KeywordList.h>

#include "VICUS_NetworkLine.h"
#include "VICUS_NetworkFluid.h"
#include "VICUS_NetworkPipe.h"
#include "VICUS_Project.h"



namespace VICUS {

Network::Network() {
	setDefaultSizingParams();
}

Network Network::copyWithBaseParameters()
{
	Network copy = *this;
	copy.m_edges.clear();
	copy.m_nodes.clear();
	return copy;
	// TODO: Hauke  implement this later for better performance
//	copy.m_id = orig.m_id;
//	copy.m_name = orig.m_name;
//	copy.m_type = orig.m_type;
//	copy.m_origin = orig.m_origin;
//	copy.m_extends = orig.m_extends;
//	copy.m_fluidID = orig.m_fluidID;
//	copy.m_scaleEdges = orig.m_scaleEdges;
//	copy.m_scaleNodes = orig.m_scaleNodes;
//	copy.m_sizingPara = orig.m_sizingPara;
//	copy.availablePipes = orig.availablePipes;

}

unsigned Network::addNode(const IBKMK::Vector3D &v, const NetworkNode::NodeType type, const bool consistentCoordinates) {

	// if there is an existing node with identical coordinates, return its id and dont add a new one
	if (consistentCoordinates){
		for (NetworkNode n: m_nodes){
			if (n.m_position.distanceTo(v) < geometricResolution)
				return n.m_id;
		}
	}
	unsigned id = m_nodes.size();
	m_nodes.push_back(NetworkNode(id, type, v));

	updateNodeEdgeConnectionPointers();

	return id;
}


unsigned Network::addNode(const NetworkNode &node, const bool considerCoordinates) {
	unsigned id = addNode(node.m_position, node.m_type, considerCoordinates);
	m_nodes[id].m_componentId = node.m_componentId;
	m_nodes[id].m_subNetworkId = node.m_subNetworkId;
//	for (unsigned n=0; n<NANDRAD::HydraulicNetworkElement::NUM_HP; ++n)
//		m_nodes[id].m_heatExchangePara[n] = node.m_heatExchangePara[n];
	m_nodes[id].m_maxHeatingDemand = node.m_maxHeatingDemand;
	return id;
}


void Network::addEdge(const unsigned nodeId1, const unsigned nodeId2, const bool supply) {
	IBK_ASSERT(nodeId1<m_nodes.size() && nodeId2<m_nodes.size());
	NetworkEdge e(nodeId1, nodeId2, supply, 0, INVALID_ID);
	m_edges.push_back(e);
	// TODO : does this needs to be done very time a node is added? or manually, when we are done?
	updateNodeEdgeConnectionPointers();
	m_edges.back().setLengthFromCoordinates();
}


void Network::addEdge(const NetworkEdge &edge) {
	IBK_ASSERT(edge.nodeId1()<m_nodes.size() && edge.nodeId2()<m_nodes.size());
	m_edges.push_back(edge);
	// TODO : does this needs to be done very time a node is added? or manually, when we are done?
	updateNodeEdgeConnectionPointers();
}


void Network::updateNodeEdgeConnectionPointers() {
	// resolve all node and edge pointers

	// first clear edge pointers in all nodes
	for (NetworkNode & n : m_nodes)
		n.m_edges.clear();

	const unsigned int nodeCount = m_nodes.size();
	// loop over all edges
	for (NetworkEdge & e : m_edges) {
		// store pointers to connected nodes
		IBK_ASSERT(e.nodeId1() < nodeCount);
		e.m_node1 = &m_nodes[e.nodeId1()];
		IBK_ASSERT(e.nodeId2() < nodeCount);
		e.m_node2 = &m_nodes[e.nodeId2()];

		// now also store pointer to this edge into connected nodes
		e.m_node1->m_edges.push_back(&e);
		e.m_node2->m_edges.push_back(&e);
	}

	// finally, also update all VICUS::Object data members
	m_children.clear();
	for (NetworkEdge & e : m_edges) {
		e.m_parent = this;
		m_children.push_back(&e);
	}
	for (NetworkNode & n : m_nodes) {
		n.m_parent = this;
		m_children.push_back(&n);
	}

}


void Network::updateVisualizationData(const VICUS::Database<VICUS::NetworkPipe> & pipeDB) {

	updateNodeEdgeConnectionPointers();

	// process all edges and update their display radius
	for (VICUS::NetworkEdge & e : m_edges) {
		double radius = 0.5;
		if (e.m_pipeId != VICUS::INVALID_ID){
			const VICUS::NetworkPipe * pipe = pipeDB[e.m_pipeId];
			if (pipe != nullptr)
				radius *= pipe->m_diameterOutside/1000 * m_scaleEdges;
		}
		e.m_visualizationRadius = radius;
	}

	// process all nodes and update their display color and radius
	for (VICUS::NetworkNode & no : m_nodes) {
		// color
		QColor color("#0e4355");
		switch (no.m_type) {
			case VICUS::NetworkNode::NT_Source:
				color = Qt::green;
			break;
			case VICUS::NetworkNode::NT_Building:
				color = Qt::blue;
			break;
			case VICUS::NetworkNode::NT_Mixer:
				color = Qt::darkGray;
			break;
			default:;
		}
		no.m_visualizationColor = color;

		// radius

		// default radius = 1 cm
		double radius = 1 * m_scaleNodes / 100; /// TODO : why not have scalenodes in same order of magnitude as scaleEdges?
		switch (no.m_type) {
			case NetworkNode::NT_Building: {
				// scale node by heating demand - 1 mm / 1000 W; 4800 W -> 48 * 0.01 = radius = 0.48
				if (no.m_maxHeatingDemand > 0)
					radius *= no.m_maxHeatingDemand / 1000;
			} break;
			case NetworkNode::NT_Source:
			case NetworkNode::NT_Mixer: {
				// if we have connected pipes, compute max radius of adjacent pipes (our node should be larger than the pipes)
				for (const VICUS::NetworkEdge * edge: no.m_edges)
					radius = std::max(radius, edge->m_visualizationRadius*1.2); // enlarge by 20 %  over edge diameter
			} break;
			default:;
		}

		// store values
		no.m_visualizationRadius = radius;
	}
}


bool Network::checkConnectedGraph() const {
	if (m_edges.size()==0 || m_nodes.size()==0)
		return false;

	std::set<const NetworkNode*> connectedNodes;
	std::set<const NetworkEdge*> connectedEdge;

	// start by any node
	const NetworkEdge * start = &m_edges[0];

	// ask edge to check its nodes
	start->collectConnectedNodes(connectedNodes, connectedEdge);

	return (connectedEdge.size() && m_edges.size() && connectedNodes.size() == m_nodes.size());
}


void Network::readGridFromCSV(const IBK::Path &filePath){
	std::vector<std::string> cont;
	IBK::FileReader::readAll(filePath, cont, std::vector<std::string>());

	// extract vector of string-xy-pairs
	std::vector<std::string> tokens;
	for (std::string line: cont){
		if (line.find("MULTILINESTRING ((") == std::string::npos)
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
			double x = IBK::string2val<double>(xyStr[0]);
			double y = IBK::string2val<double>(xyStr[1]);
			polyLine.push_back({x, y});
		}
		for (unsigned i=0; i<polyLine.size()-1; ++i){
			unsigned n1 = addNodeExt(IBKMK::Vector3D(polyLine[i][0], polyLine[i][1], 0), NetworkNode::NT_Mixer);
			unsigned n2 = addNodeExt(IBKMK::Vector3D(polyLine[i+1][0], polyLine[i+1][1], 0), NetworkNode::NT_Mixer);
			addEdge(n1, n2, true);
		}
	}
}


void Network::readBuildingsFromCSV(const IBK::Path &filePath, const double &heatDemand) {
	std::vector<std::string> cont;
	IBK::FileReader::readAll(filePath, cont, std::vector<std::string>());

	// extract vector of string-xy
	std::vector<std::string> lineSepStr;
	std::vector<std::string> xyStr;
	for (std::string line: cont){
		if (line.find("POINT") == std::string::npos)
			continue;
		IBK::explode(line, lineSepStr, ",", IBK::EF_NoFlags);
		IBK::trim(lineSepStr[0], "\"");
		IBK::trim(lineSepStr[0], "POINT ((");
		IBK::trim(lineSepStr[0], "))");
		IBK::explode(lineSepStr[0], xyStr, " ", IBK::EF_NoFlags);
		if (xyStr.size()!=2)
			continue;
		// add node
		unsigned id = addNodeExt(IBKMK::Vector3D(IBK::string2val<double>(xyStr[0]), IBK::string2val<double>(xyStr[1]), 0), NetworkNode::NT_Building);
		m_nodes[id].m_maxHeatingDemand = heatDemand;
	}
}


void Network::assignSourceNode(const IBKMK::Vector3D &v) {
	IBK_ASSERT(!m_nodes.empty());
	NetworkNode * nMin = nullptr;
	double distMin = std::numeric_limits<double>::max();
	for (NetworkNode &n: m_nodes){
		double dist = n.m_position.distanceTo(v - m_origin);
		if (dist < distMin){
			distMin = dist;
			nMin = &n;
		}
	}

	nMin->m_type = NetworkNode::NT_Source;
}


void Network::generateIntersections(){
	while (findAndAddIntersection()) {}
}


bool Network::findAndAddIntersection() {

	for (unsigned i1=0; i1<m_edges.size(); ++i1) {
		for (unsigned i2=i1+1; i2<m_edges.size(); ++i2) {

			// calculate intersection
			NetworkLine2D l1 = NetworkLine2D(m_edges[i1]);
			NetworkLine2D l2 = NetworkLine2D(m_edges[i2]);
			IBK::point2D<double> ps;
			l1.intersection(l2, ps);

			// if it is within both lines: add node and edges, adapt exisiting nodes
			if (l1.containsPoint(ps) && l2.containsPoint(ps)){
				unsigned nInter = addNode(IBKMK::Vector3D(ps), NetworkNode::NT_Mixer);
				addEdge(nInter, m_edges[i1].nodeId1(), true);
				addEdge(nInter, m_edges[i2].nodeId1(), true);
				m_edges[i1].setNodeId1(nInter, &m_nodes[nInter]);
				m_edges[i2].setNodeId1(nInter, &m_nodes[nInter]);
				updateNodeEdgeConnectionPointers();
				return true;
			}
		}
	}
	return false;
}


void Network::connectBuildings(const bool extendSupplyPipes) {

	int idNext = nextUnconnectedBuilding();
	while (idNext>=0) {

		unsigned idBuilding = static_cast<unsigned>(idNext);

		// find closest supply edge
		double distMin = std::numeric_limits<double>::max();
		unsigned idEdgeMin=0;
		for (unsigned id=0; id<m_edges.size(); ++id){
			if (!m_edges[id].m_supply)
				continue;
			double dist = NetworkLine2D(m_edges[id]).distanceToPoint(m_nodes[idBuilding].m_position.point2D());
			if (dist<distMin){
				distMin = dist;
				idEdgeMin = id;
			}
		}
		// branch node
		NetworkLine2D lMin = NetworkLine2D(m_edges[idEdgeMin]);
		IBK::point2D<double> pBranch;
		unsigned idBranch;
		lMin.projectionFromPoint(m_nodes[idBuilding].m_position.point2D(), pBranch);
		// branch node is inside edge: split edge
		if (lMin.containsPoint(pBranch)){
			idBranch = addNode(IBKMK::Vector3D(pBranch), NetworkNode::NT_Mixer);
			addEdge(m_edges[idEdgeMin].nodeId1(), idBranch, true);
			m_edges[idEdgeMin].setNodeId1(idBranch, &m_nodes[idBranch]);
			updateNodeEdgeConnectionPointers();
		}
		// branch node is outside edge
		else{
			double dist1 = NetworkLine2D::distanceBetweenPoints(pBranch, m_edges[idEdgeMin].m_node1->m_position.point2D());
			double dist2 = NetworkLine2D::distanceBetweenPoints(pBranch, m_edges[idEdgeMin].m_node2->m_position.point2D());
			idBranch = (dist1 < dist2) ? m_edges[idEdgeMin].nodeId1() : m_edges[idEdgeMin].nodeId2();
			// if pipe should be extended, change coordinates of branch node
			if (extendSupplyPipes){
				m_nodes[idBranch].m_position = pBranch;
				for (NetworkEdge *e: m_nodes[idBranch].m_edges)
					e->setLengthFromCoordinates();
			}
		}
		// connect building to branch node
		addEdge(idBranch, idBuilding, false);

		idNext = nextUnconnectedBuilding();
	}
}


int Network::nextUnconnectedBuilding() const{
	for (const NetworkNode &nBuilding: m_nodes){
		if (nBuilding.m_type == NetworkNode::NT_Building && nBuilding.m_edges.size()==0)
			return nBuilding.m_id;
	}
	return -1;
}


void Network::cleanDeadEnds(Network &cleanNetwork, const unsigned maxSteps){

	for (unsigned step=0; step<maxSteps; ++step){
		for (unsigned n=0; n<m_nodes.size(); ++n)
			m_nodes[n].updateIsDeadEnd();
	}
	for (const NetworkEdge &edge: m_edges){
		if (edge.m_node1->m_isDeadEnd || edge.m_node2->m_isDeadEnd)
			continue;
		unsigned id1 = cleanNetwork.addNode(*edge.m_node1);
		unsigned id2 = cleanNetwork.addNode(*edge.m_node2);
		cleanNetwork.addEdge(NetworkEdge(id1, id2, edge.m_supply, edge.length(), edge.m_pipeId));
	}
}


void Network::cleanRedundantEdges(Network & cleanNetwork) const{

	IBK_ASSERT(m_edges.size()>0);
	std::set<unsigned> proccessedNodes;

	for (const NetworkEdge &edge: m_edges){

		if (edge.m_node1->isRedundant() || edge.m_node2->isRedundant()){

			// proccess redundant nodes only once
			NetworkNode * redundantNode = (edge.m_node1->isRedundant()) ? edge.m_node1: edge.m_node2;
			if (proccessedNodes.find(redundantNode->m_id) != proccessedNodes.end())
				continue;
			proccessedNodes.insert(redundantNode->m_id);

			// get previous node and next non-redundant node
			NetworkNode * previousNode = edge.neighbourNode(redundantNode);
			NetworkEdge * nextEdge = redundantNode->neighborEdge(&edge);
			std::set<unsigned> redundantNodes;
			double totalLength = edge.length();
			const NetworkNode * nextNode = redundantNode->findNextNonRedundantNode(redundantNodes, totalLength, nextEdge);
			for (const unsigned nId: redundantNodes)
				proccessedNodes.insert(nId);

			// add nodes and reduced edge to new network
			unsigned id1 = cleanNetwork.addNode(*previousNode);
			unsigned id2 = cleanNetwork.addNode(*nextNode);
			cleanNetwork.addEdge(NetworkEdge(id1, id2, edge.m_supply, totalLength, edge.m_pipeId));
		}
		else{
			unsigned id1 = cleanNetwork.addNode(*edge.m_node1);
			unsigned id2 = cleanNetwork.addNode(*edge.m_node2);
			cleanNetwork.addEdge(NetworkEdge(id1, id2, edge.m_supply, edge.length(), edge.m_pipeId));
		}
	}
}


void Network::cleanShortEdges(Network &cleanNetwork, const double &threshold)
{
	IBK_ASSERT(m_edges.size()>0);

	for (const NetworkEdge &edge: m_edges){

		if (edge.length() > threshold){
			unsigned id1 = cleanNetwork.addNode(*edge.m_node1);
			unsigned id2 = cleanNetwork.addNode(*edge.m_node2);
			cleanNetwork.addEdge(NetworkEdge(id1, id2, edge.m_supply, edge.length(), edge.m_pipeId));
		}
	}
}


void Network::sizePipeDimensions(const NetworkFluid *fluid, std::vector<const VICUS::NetworkPipe*> & availablePipes) {
FUNCID(Network::sizePipeDimensions);

	updateNodeEdgeConnectionPointers();

	// check pipe database
	if (availablePipes.empty())
		throw IBK::Exception(IBK::FormatString("The pipe database of network '%1' is empty."
												"Please add pipes to this network").arg(m_id), FUNC_ID);

	// check parameters
	for (unsigned int n = 0; n < NUM_SP; ++n){
		if (m_sizingPara[n].empty())
			throw IBK::Exception(IBK::FormatString("'%1' not set").arg(VICUS::KeywordList::Keyword("Network::sizingParam", n)), FUNC_ID);
	}

	// check for source
	std::vector<NetworkNode> sources;
	findSourceNodes(sources);
	if (sources.size() < 1)
		throw IBK::Exception("Network has no source node. Set one node to type source.", FUNC_ID);

	// set all edges heating demand = 0
	for (NetworkEdge &edge: m_edges)
		edge.m_maxHeatingDemand = 0;

	// for all buildings: add their heating demand to the pipes along their shortest path
	for (NetworkNode &node: m_nodes) {
		if (node.m_type != NetworkNode::NT_Building)
			continue;
		if (node.m_maxHeatingDemand <= 0)
			throw IBK::Exception(IBK::FormatString("Maximum heating demand of node '%1' must be >0").arg(node.m_id), FUNC_ID);

		// for each source find the shortest path to current node. Finally select the shortest of these paths
		std::vector<NetworkEdge * > minPath;
		double minPathLength = std::numeric_limits<double>::max();
		for (const NetworkNode &source: sources){
			std::vector<NetworkEdge * > path;
			dijkstraShortestPathToSource(node, source, path);  // shortest path between source and node
			double pathLength = 0;
			for (NetworkEdge *edge: path)
				pathLength += edge->length();
			if (pathLength < minPathLength){
				minPathLength = pathLength;
				minPath = path;
			}
		}
		for (NetworkEdge * edge: minPath)
			edge->m_maxHeatingDemand += node.m_maxHeatingDemand;
	}

	// in case there is a pipe which is not part of any path (e.g. in circular grid): assign the adjacent heating demand
	for (NetworkEdge &e: m_edges){
		if (e.m_maxHeatingDemand <= 0){
			std::set<NetworkEdge *> edges1, edges2;
			e.m_maxHeatingDemand = 0.5 * ( e.m_node1->adjacentHeatingDemand(edges1)
										+ e.m_node2->adjacentHeatingDemand(edges2) );
		}
	}

	// determine pipe with largest inner diameter
	const NetworkPipe * largestPipe = availablePipes[0];
	for (const NetworkPipe * pipe: availablePipes) {
		if (pipe->diameterInside() > largestPipe->diameterInside())
			largestPipe = pipe;
	}


	// for each edge: find the smallest pipe from DB that has a pressure loss below deltapMax
	double deltaPMax = m_sizingPara[SP_MaxPressureLoss].get_value("Pa/m");
	for (NetworkEdge &e: m_edges) {
		const NetworkPipe * currentPipe = nullptr; // here we store the candidate for the pipe to use

		for (const NetworkPipe * pipe : availablePipes){
			double massFlow = e.m_maxHeatingDemand / (m_sizingPara[SP_TemperatureDifference].get_value("K")
													  * fluid->m_para[NetworkFluid::P_HeatCapacity].get_value("J/kgK"));
			//  pressure loss per length (Pa/m)
			double dp = pressureLossColebrook(1.0, massFlow, fluid, *pipe, m_sizingPara[SP_TemperatureSetpoint].get_value("C"));
			// select smallest possible pipe
			if (dp < deltaPMax){
				// if still unset, use this pipe
				if (currentPipe == nullptr)
					currentPipe = pipe;
				// otherwise compare inside diameter and only take if smaller
				else if (pipe->diameterInside() < currentPipe->diameterInside())
					currentPipe = pipe;
			}
		}

		// if we found a pipe, store its id
		if (currentPipe != nullptr)
			e.m_pipeId = currentPipe->m_id;
		// otherwise store ID of biggest pipe
		else
			e.m_pipeId = largestPipe->m_id;
	}

}


void Network::findSourceNodes(std::vector<NetworkNode> &sources) const{
	for (NetworkNode n: m_nodes){
		if (n.m_type==NetworkNode::NT_Source)
			sources.push_back(n);
	}
}


void Network::dijkstraShortestPathToSource(NetworkNode &startNode, const NetworkNode &endNode,
										   std::vector<NetworkEdge*> &pathEndToStart){

	// init: all nodes have infinte distance to start node and no predecessor
	for (NetworkNode &n: m_nodes){
		n.m_distanceToStart = std::numeric_limits<double>::max();
		n.m_predecessor = nullptr;
	}
	startNode.m_distanceToStart = 0;
	std::set<unsigned> visitedNodes;

	// go through all not-visited nodes
	while (visitedNodes.size() <= m_nodes.size()){
		// find node with currently smallest distance to start, which has not yet been visited:
		double minDistance = std::numeric_limits<double>::max();
		NetworkNode *nMin = nullptr;
		for (unsigned id = 0; id < m_nodes.size(); ++id){
			if (visitedNodes.find(id) == visitedNodes.end() && m_nodes[id].m_distanceToStart < minDistance){
				minDistance = m_nodes[id].m_distanceToStart;
				nMin = &m_nodes[id];
			}
		}
		IBK_ASSERT(nMin != nullptr);
		// if endNode reached: return path
		if (nMin->m_id == endNode.m_id){
			nMin->pathToNull(pathEndToStart);
			return;
		}
		// update distance from start to neighbours of nMin
		visitedNodes.insert(nMin->m_id);
		nMin->updateNeighbourDistances();
	}
}


void Network::updateExtends() {
	updateNodeEdgeConnectionPointers();
	double minX = std::numeric_limits<double>::max();
	double maxX = -std::numeric_limits<double>::max();
	double minY = std::numeric_limits<double>::max();
	double maxY = -std::numeric_limits<double>::max();
	// now process all nodes
	for (const VICUS::NetworkNode & node : m_nodes) {
		minX = std::min(minX, node.m_position.m_x);
		maxX = std::max(maxX, node.m_position.m_x);
		minY = std::min(minY, node.m_position.m_y);
		maxY = std::max(maxY, node.m_position.m_y);
	}
	m_extends.set(minX, minY, maxX, maxY);
}


// TODO Hauke: this will be moved to NANDRAD Solver later (with a different interface)
double Network::pressureLossColebrook(const double &length, const double &massFlow, const NetworkFluid *fluid,
										const NetworkPipe &pipe, const double &temperature){

	double velocity = massFlow / (fluid->m_para[NetworkFluid::P_Density].value * pipe.diameterInside()/1000
			* pipe.diameterInside()/1000  * 3.14159 / 4);
	double Re = velocity * pipe.diameterInside()/1000 / fluid->m_kinematicViscosity.m_values.value(temperature);
	double lambda = 0.05;
	double lambda_new = lambda;
	for (unsigned n=0; n<100; ++n){
		lambda_new = std::pow(-2 * std::log10(2.51 / (Re * std::sqrt(lambda)) + pipe.m_roughness/1000 /
											  (3.71 * pipe.diameterInside()/1000 )), -2);
		if (abs(lambda_new - lambda) / lambda < 1e-3)
			break;
		lambda = lambda_new;
	}
	return lambda_new * length / (pipe.diameterInside()/1000)  * fluid->m_para[NetworkFluid::P_Density].value
			/ 2 * velocity * velocity;
}

IBKMK::Vector3D Network::origin() const
{
	return m_origin;
}

void Network::setOrigin(const IBKMK::Vector3D &origin)
{
	m_origin = origin;
	for (NetworkNode &n: m_nodes)
		n.m_position -= m_origin;
}


double Network::totalLength() const{
	double length = 0;
	for(const NetworkEdge &e: m_edges){
		length += e.length();
	}
	return length;
}


NetworkEdge *Network::edge(unsigned nodeId1, unsigned nodeId2){
	for (NetworkEdge &e: m_edges){
		if (e.nodeId1() == nodeId1 && e.nodeId2() == nodeId2)
			return &e;
	}
	IBK_ASSERT(false);
	return nullptr;
}


double Network::numberOfBuildings() const{
	double count = 0;
	for (const NetworkNode &n: m_nodes){
		if (n.m_type == NetworkNode::NT_Building)
			++count;
	}
	return count;
}


void Network::setDefaultSizingParams() {
	m_sizingPara[Network::SizingParam::SP_TemperatureSetpoint] = IBK::Parameter("TemperatureSetpoint", 5, IBK::Unit("C"));
	KeywordList::setParameter(m_sizingPara, "Network::SizingParam", Network::SizingParam::SP_TemperatureDifference, 5);
	KeywordList::setParameter(m_sizingPara, "Network::SizingParam", Network::SizingParam::SP_MaxPressureLoss, 150);
}


void Network::writeNetworkCSV(const IBK::Path &file) const{
	std::ofstream f;
	f.open(file.str(), std::ofstream::out | std::ofstream::trunc);
	for (const NetworkEdge &e: m_edges){
		f.precision(10);
		f << std::fixed << e.m_node1->m_position.m_x << "\t" << e.m_node1->m_position.m_y << "\t"
		  << e.m_node2->m_position.m_x << "\t" << e.m_node2->m_position.m_y << "\t" << e.length() << std::endl;
	}
	f.close();
}


void Network::writePathCSV(const IBK::Path &file, const NetworkNode & node, const std::vector<NetworkEdge *> &path) const {
	std::ofstream f;
	f.open(file.str(), std::ofstream::out | std::ofstream::trunc);
	f.precision(10);
	f << std::fixed << node.m_position.m_x << "\t" << node.m_position.m_y << std::endl;
	for (const NetworkEdge *e: path){
		f << std::fixed << e->m_node1->m_position.m_x << "\t" << e->m_node1->m_position.m_y << "\t"
		  << e->m_node2->m_position.m_x << "\t" << e->m_node2->m_position.m_y << "\t" << e->length() << std::endl;
	}
	f.close();
}


void Network::writeBuildingsCSV(const IBK::Path &file) const {
	std::ofstream f;
	f.open(file.str(), std::ofstream::out | std::ofstream::trunc);
	f.precision(10);
	for (const NetworkNode &n: m_nodes){
		if (n.m_type==NetworkNode::NT_Building)
			f << std::fixed << n.m_position.m_x << "\t" << n.m_position.m_y << "\t" << n.m_maxHeatingDemand << std::endl;
	}
	f.close();
}



} // namespace VICUS
