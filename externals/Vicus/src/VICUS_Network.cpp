#include "VICUS_Network.h"
#include "VICUS_NetworkLine.h"
#include "VICUS_NetworkFluid.h"
#include "VICUS_NetworkPipe.h"

#include <IBK_assert.h>
#include <IBK_Path.h>
#include <IBK_FileReader.h>

#include <fstream>
#include <algorithm>


namespace VICUS {

Network::Network() {

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
	return addNode(node.m_position, node.m_type, considerCoordinates);
}


void Network::addEdge(const unsigned nodeId1, const unsigned nodeId2, const bool supply) {
	IBK_ASSERT(nodeId1<m_nodes.size() && nodeId2<m_nodes.size());
	m_edges.push_back(NetworkEdge(nodeId1, nodeId2, supply));
	// TODO : does this needs to be done very time a node is added? or manually, when we are done?
	updateNodeEdgeConnectionPointers();
}


void Network::addEdge(const NetworkEdge &edge) {
	IBK_ASSERT(edge.m_nodeId1<m_nodes.size() && edge.m_nodeId2<m_nodes.size());
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
		IBK_ASSERT(e.m_nodeId1 < nodeCount);
		e.m_node1 = &m_nodes[e.m_nodeId1];
		IBK_ASSERT(e.m_nodeId2 < nodeCount);
		e.m_node2 = &m_nodes[e.m_nodeId2];

		// now also store pointer to this edge into connected nodes
		e.m_node1->m_edges.push_back(&e);
		e.m_node2->m_edges.push_back(&e);
	}
}


bool Network::checkConnectedGraph() const {
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
	std::vector<std::string> xyStr;
	for (std::string line: cont){
		if (line.find("POINT") == std::string::npos)
			continue;
		IBK::trim(line, ",");
		IBK::trim(line, "\"");
		IBK::trim(line, "POINT ((");
		IBK::trim(line, "))");
		IBK::explode(line, xyStr, " ", IBK::EF_NoFlags);
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
				addEdge(nInter, m_edges[i1].m_nodeId1, true);
				addEdge(nInter, m_edges[i2].m_nodeId1, true);
				m_edges[i1].m_nodeId1 = nInter;
				m_edges[i2].m_nodeId1 = nInter;
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
			addEdge(m_edges[idEdgeMin].m_nodeId1, idBranch, true);
			m_edges[idEdgeMin].m_nodeId1 = idBranch;
			updateNodeEdgeConnectionPointers();
		}
		// branch node is outside edge
		else{
			double dist1 = NetworkLine2D::distanceBetweenPoints(pBranch, m_edges[idEdgeMin].m_node1->m_position.point2D());
			double dist2 = NetworkLine2D::distanceBetweenPoints(pBranch, m_edges[idEdgeMin].m_node2->m_position.point2D());
			idBranch = (dist1 < dist2) ? m_edges[idEdgeMin].m_nodeId1 : m_edges[idEdgeMin].m_nodeId2 ;
			// if pipe should be extended, change coordinates of branch node
			if (extendSupplyPipes){
				m_nodes[idBranch].m_position = pBranch;
				updateNodeEdgeConnectionPointers();
			}
		}
		// connect building to branch node
		addEdge(idBranch, idBuilding, false);

		idNext = nextUnconnectedBuilding();
	}
}


int Network::nextUnconnectedBuilding(){
	for (NetworkNode &nBuilding: m_nodes){
		if (nBuilding.m_type == NetworkNode::NT_Building && nBuilding.m_edges.size()==0)
			return nBuilding.m_id;
	}
	return -1;
}


void Network::networkWithoutDeadEnds(Network &cleanNetwork, const unsigned maxSteps){

	for (unsigned step=0; step<maxSteps; ++step){
		for (unsigned n=0; n<m_nodes.size(); ++n){
			m_nodes[n].updateIsDeadEnd();
		}
	}
	for (const NetworkEdge &e: m_edges){
		if (e.m_node1->m_isDeadEnd || e.m_node2->m_isDeadEnd)
			continue;
		unsigned id1 = cleanNetwork.addNode(*e.m_node1);
		unsigned id2 = cleanNetwork.addNode(*e.m_node2);
		cleanNetwork.addEdge(NetworkEdge(id1, id2, e.m_length, e.m_diameterInside, e.m_supply));
	}
}


void Network::calculateLengths(){
	for (NetworkEdge &e: m_edges) {
		e.m_length = NetworkLine2D(e).length();
	}
}


void Network::networkWithReducedEdges(Network & reducedNetwork){

	IBK_ASSERT(m_edges.size()>0);
	std::set<unsigned> proccessedNodes;

	for (NetworkEdge &edge: m_edges){

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
			double totalLength = edge.m_length;
			const NetworkNode * nextNode = redundantNode->findNextNonRedundantNode(redundantNodes, totalLength, nextEdge);
			for (const unsigned nId: redundantNodes)
				proccessedNodes.insert(nId);

			// add nodes and reduced edge to new network
			unsigned id1 = reducedNetwork.addNode(*previousNode);
			unsigned id2 = reducedNetwork.addNode(*nextNode);
			reducedNetwork.addEdge(NetworkEdge(id1, id2, totalLength, edge.m_diameterInside, edge.m_supply));
		}
		else{
			unsigned id1 = reducedNetwork.addNode(*edge.m_node1);
			unsigned id2 = reducedNetwork.addNode(*edge.m_node2);
			reducedNetwork.addEdge(NetworkEdge(id1, id2, edge.m_length, edge.m_diameterInside, edge.m_supply));
		}
	}
}


void Network::sizePipeDimensions(const double &deltaPMax, const double &deltaTemp, const double &temp,
								 const NetworkFluid &fluid, const std::vector<NetworkPipe> &pipeDB){

	// for all buildings: add their heating demand to the pipes along their path
	for (NetworkNode &node: m_nodes) {
		std::vector<NetworkEdge * > path;
		if (node.m_type == NetworkNode::NT_Building){
			dijkstraShortestPathToSource(node, path);
			for (NetworkEdge * edge: path)
				edge->m_heatingDemand += node.m_maxHeatingDemand;
		}
	}

	// in case there is a pipe which is not part of any path (e.g. in circular grid): assign the adjacent heating demand
	for (NetworkEdge &e: m_edges){
		if (e.m_heatingDemand <= 0){
			std::set<NetworkEdge *> edges1, edges2;
			e.m_heatingDemand = 0.5 * ( e.m_node1->adjacentHeatingDemand(edges1)
										+ e.m_node2->adjacentHeatingDemand(edges2) );
		}
	}

	for (NetworkEdge &e: m_edges){
		for (const NetworkPipe &pipe: pipeDB){
			double massFlow = e.m_heatingDemand / deltaTemp / fluid.m_para[NetworkFluid::P_HeatCapacity].value;
			if (pressureLossColebrook(e.m_length, massFlow, fluid, pipe, temp) < deltaPMax){
				e.m_diameterOutside = pipe.m_diameterOutside;
				e.m_diameterInside = pipe.m_diameterInside();
				break;
			}
		}
	}
}


void Network::dijkstraShortestPathToSource(NetworkNode &startNode, std::vector<NetworkEdge*> &pathSourceToStart){

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
		// if source reached: return path
		if (nMin->m_type == NetworkNode::NT_Source){
			nMin->pathToNull(pathSourceToStart);
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


double Network::pressureLossColebrook(const double &length, const double &massFlow, const NetworkFluid &fluid,
										const NetworkPipe &pipe, const double &temperature){

	double velocity = massFlow / (fluid.m_para[NetworkFluid::P_Density].value * pipe.m_diameterInside() * pipe.m_diameterInside() * 3.14159 / 4);
	double Re = velocity * pipe.m_diameterInside() / fluid.m_kinematicViscosity.m_values.value(temperature);
	double lambda = 0.05;
	double lambda_new = lambda;
	for (unsigned n=0; n<100; ++n){
		lambda_new = std::pow(-2 * std::log10(2.51 / (Re * std::sqrt(lambda)) + pipe.m_roughness / (3.71 * pipe.m_diameterInside())), -2);
		if (abs(lambda_new - lambda) / lambda < 1e-3)
			break;
		lambda = lambda_new;
	}
	return lambda_new * length / pipe.m_diameterInside() * fluid.m_para[NetworkFluid::P_Density].value / 2 * velocity * velocity;
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


double Network::totalLength()
{
	double length = 0;
	for(NetworkEdge &e: m_edges){
		length += e.m_length;
	}
	return length;
}


void Network::writeNetworkCSV(const IBK::Path &file) const{
	std::ofstream f;
	f.open(file.str(), std::ofstream::out | std::ofstream::trunc);
	for (const NetworkEdge &e: m_edges){
		f.precision(10);
		f << std::fixed << e.m_node1->m_position.m_x << "\t" << e.m_node1->m_position.m_y << "\t"
		  << e.m_node2->m_position.m_x << "\t" << e.m_node2->m_position.m_y << "\t" << e.m_length << std::endl;
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
		  << e->m_node2->m_position.m_x << "\t" << e->m_node2->m_position.m_y << "\t" << e->m_length << std::endl;
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
