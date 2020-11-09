#include "VICUS_Network.h"
#include "VICUS_NetworkLine.h"

#include <IBK_assert.h>
#include <IBK_Path.h>
#include <IBK_FileReader.h>

#include <fstream>
#include <algorithm>


namespace VICUS {

Network::Network() {

}


unsigned Network::addNode(const double &x, const double &y, const Node::NodeType type, const bool consistentCoordinates) {
	// if there is an existing node with identical coordinates, return its id and dont add a new one
	if (consistentCoordinates){
		for (Node n: m_nodes){
			if (Line::pointsMatch(n.m_x, n.m_y, x, y))
				return n.m_id;
		}
	}
	unsigned id = m_nodes.size();
	m_nodes.push_back(Node(id, x, y, type));

	updateNodeEdgeConnectionPointers();

	return id;
}


unsigned Network::addNode(const Node &node, const bool considerCoordinates) {
	return addNode(node.m_x, node.m_y, node.m_type, considerCoordinates);
}


void Network::addEdge(const unsigned nodeId1, const unsigned nodeId2, const bool supply) {
	IBK_ASSERT(nodeId1<m_nodes.size() && nodeId2<m_nodes.size());
	m_edges.push_back(Edge(nodeId1, nodeId2, supply));
	// TODO : does this needs to be done very time a node is added? or manually, when we are done?
	updateNodeEdgeConnectionPointers();
}


void Network::addEdge(const Edge &edge) {
	IBK_ASSERT(edge.m_nodeId1<m_nodes.size() && edge.m_nodeId2<m_nodes.size());
	m_edges.push_back(edge);
	// TODO : does this needs to be done very time a node is added? or manually, when we are done?
	updateNodeEdgeConnectionPointers();
}


void Network::updateNodeEdgeConnectionPointers() {
	// resolve all node and edge pointers

	// first clear edge pointers in all nodes
	for (Node & n : m_nodes)
		n.m_edges.clear();

	const unsigned int nodeCount = m_nodes.size();
	// loop over all edges
	for (Edge & e : m_edges) {
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
	std::set<const Node*> connectedNodes;
	std::set<const Edge*> connectedEdge;

	// start by any node
	const Edge * start = &m_edges[0];

	// ask edge to check its nodes
	start->collectConnectedNodes(connectedNodes, connectedEdge);

	return (connectedEdge.size() && m_edges.size() && connectedNodes.size() == m_nodes.size());
}


void Network::readCSV(const IBK::Path &filePath, std::vector<std::string> &content) {
	FUNCID(Network::readCSV);

	// TODO : see FileReader from IBK lib

	if (!filePath.exists())
		throw IBK::Exception(IBK::FormatString("File '%1' doesn't exist.").arg(filePath), FUNC_ID);

#if defined(_WIN32)
#if defined(_MSC_VER)
	std::ifstream file(filePath.wstr().c_str(), std::ios_base::app);
#else
	std::string filenameAnsi = IBK::WstringToANSI(filePath.wstr(), false);
	std::ifstream file(filenameAnsi.c_str(), std::ios_base::app);
#endif
#else
	std::ifstream file(filePath.c_str(), std::ios_base::app);
#endif

	std::string line;
	content.clear();
	while (std::getline(file, line))
		content.push_back(line);
	file.close();
}


void Network::readGridFromCSV(const IBK::Path &filePath){

	// TODO : use FileReader from IBK lib

	std::vector<std::string> cont;
	//	readCSV(filePath, cont);

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
			polyLine.push_back(std::vector<double> {IBK::string2val<double>(xyStr[0]),
													IBK::string2val<double>(xyStr[1])});
		}
		for (unsigned i=0; i<polyLine.size()-1; ++i){
			unsigned n1 = addNode(polyLine[i][0], polyLine[i][1], Node::NT_Mixer);
			unsigned n2 = addNode(polyLine[i+1][0], polyLine[i+1][1], Node::NT_Mixer);
			addEdge(n1, n2, true);
		}
	}
}


void Network::readBuildingsFromCSV(const IBK::Path &filePath, const double &heatDemand) {
	std::vector<std::string> cont;
	readCSV(filePath, cont);

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
		unsigned id = addNode(IBK::string2val<double>(xyStr[0]), IBK::string2val<double>(xyStr[1]), Node::NT_Building);
		m_nodes[id].m_heatingDemand = heatDemand;
	}
}


void Network::setSource(const double &x, const double &y) {
	IBK_ASSERT(!m_nodes.empty());
	Node * nMin = nullptr;
	double distMin = std::numeric_limits<double>::max();
	for (Node &n: m_nodes){
		double dist = Line::distanceBetweenPoints(x, y, n.m_x, n.m_y);
		if (dist < distMin){
			distMin = dist;
			nMin = &n;
		}
	}

	nMin->m_type = Node::NT_Source;
}


void Network::generateIntersections(){
	// TODO : clarify "deterministic result independent of initial node/edge storage order"
	while (findAndAddIntersection()) {}
}


bool Network::findAndAddIntersection() {

	for (unsigned i1=0; i1<m_edges.size(); ++i1) {
		for (unsigned i2=i1+1; i2<m_edges.size(); ++i2) {

			// calculate intersection and

			Line l1 = Line(m_edges[i1]);
			Line l2 = Line(m_edges[i2]);
			double xs, ys;
			l1.intersection(l2, xs, ys);

			// if it is within both lines: add node and edges, adapt exisiting nodes
			if (l1.containsPoint(xs, ys) && l2.containsPoint(xs, ys)){
				unsigned nInter = addNode(xs, ys, Node::NT_Mixer);
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
			double dist = Line(m_edges[id]).distanceToPoint(m_nodes[idBuilding].m_x, m_nodes[idBuilding].m_y);
			if (dist<distMin){
				distMin = dist;
				idEdgeMin = id;
			}
		}
		// branch node
		Line lMin = Line(m_edges[idEdgeMin]);
		double xBranch, yBranch;
		unsigned idBranch;
		lMin.projectionFromPoint(m_nodes[idBuilding].m_x, m_nodes[idBuilding].m_y, xBranch, yBranch);

		// branch node is inside edge: split edge
		if (lMin.containsPoint(xBranch,yBranch)){
			idBranch = addNode(xBranch, yBranch, Node::NT_Mixer);
			addEdge(m_edges[idEdgeMin].m_nodeId1, idBranch, true);
			m_edges[idEdgeMin].m_nodeId1 = idBranch;
			updateNodeEdgeConnectionPointers();
		}
		// branch node is outside edge
		else{
			double dist1 = Line::distanceBetweenPoints(xBranch, yBranch, m_edges[idEdgeMin].m_node1->m_x,
													   m_edges[idEdgeMin].m_node1->m_y);
			double dist2 = Line::distanceBetweenPoints(xBranch, yBranch, m_edges[idEdgeMin].m_node2->m_x,
													   m_edges[idEdgeMin].m_node2->m_y);
			idBranch = (dist1 < dist2) ? m_edges[idEdgeMin].m_nodeId1 : m_edges[idEdgeMin].m_nodeId2 ;
			// if pipe should be extended, change coordinates of branch node
			if (extendSupplyPipes){
				m_nodes[idBranch].m_x = xBranch;
				m_nodes[idBranch].m_y = yBranch;
				updateNodeEdgeConnectionPointers();
			}
		}
		// connect building to branch node
		addEdge(idBranch, idBuilding, false);

		idNext = nextUnconnectedBuilding();
	}
}


int Network::nextUnconnectedBuilding(){
	for (Node &nBuilding: m_nodes){
		if (nBuilding.m_type == Node::NT_Building && nBuilding.m_edges.size()==0)
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
	for (const Edge &e: m_edges){
		if (e.m_node1->m_isDeadEnd || e.m_node2->m_isDeadEnd)
			continue;
		unsigned id1 = cleanNetwork.addNode(*e.m_node1);
		unsigned id2 = cleanNetwork.addNode(*e.m_node2);
		cleanNetwork.addEdge(Edge(id1, id2, e.m_length, e.m_diameter, e.m_supply));
	}
}


void Network::calculateLengths(){
	for (Edge &e: m_edges) {
		e.m_length = Line(e).length();
	}
}


void Network::sizePipeDimensions(const double &dpMax, const double &dT, const double &fluidDensity,
								 const double &fluidKinViscosity, const double &roughness){

	// for all buildings: add their heating demand to the pipes along their path
	for (Node &node: m_nodes) {
		std::vector<Edge * > path;
		if (node.m_type == Node::NT_Building){
			dijkstraShortestPathToSource(node, path);
			for (Edge * edge: path)
				edge->m_heatingDemand += node.m_heatingDemand;
		}
	}

	// in case there is a pipe which is not part of any path (e.g. in circular grid): assign the adjacent heating demand
	for (Edge &e: m_edges){
		if (e.m_heatingDemand <= 0){
			std::set<Edge *> edges1, edges2;
			e.m_heatingDemand = 0.5 * ( e.m_node1->adjacentHeatingDemand(edges1)
										+ e.m_node2->adjacentHeatingDemand(edges2) );
		}
	}

	// we need a table with pipe dimensions here
	// and an interface to the fluid properties...

	for (Edge &e: m_edges){
		double cp =  3800;
		double massFlow = e.m_heatingDemand / dT / cp;
		double dp = pressureLossColebrook(0.01, e.m_length, roughness, massFlow, fluidDensity, fluidKinViscosity);
	}
}


void Network::networkWithReducedEdges(Network & reducedNetwork){

	IBK_ASSERT(m_edges.size()>0);
	std::set<unsigned> proccessedNodes;

	for (Edge &edge: m_edges){

		if (edge.m_node1->isRedundant() || edge.m_node2->isRedundant()){

			// proccess redundant nodes only once
			Node * redundantNode = (edge.m_node1->isRedundant()) ? edge.m_node1: edge.m_node2;
			if (proccessedNodes.find(redundantNode->m_id) != proccessedNodes.end())
				continue;
			proccessedNodes.insert(redundantNode->m_id);

			// get previous node and next non-redundant node
			Node * previousNode = edge.neighbourNode(redundantNode);
			Edge * nextEdge = redundantNode->neighborEdge(&edge);
			std::set<unsigned> redundantNodes;
			double totalLength = edge.m_length;
			const Node * nextNode = redundantNode->findNextNonRedundantNode(redundantNodes, totalLength, nextEdge);
			for (const unsigned nId: redundantNodes)
				proccessedNodes.insert(nId);

			// add nodes and reduced edge to new network
			unsigned id1 = reducedNetwork.addNode(*previousNode);
			unsigned id2 = reducedNetwork.addNode(*nextNode);
			reducedNetwork.addEdge(Edge(id1, id2, totalLength, edge.m_diameter, edge.m_supply));
		}
		else{
			unsigned id1 = reducedNetwork.addNode(*edge.m_node1);
			unsigned id2 = reducedNetwork.addNode(*edge.m_node2);
			reducedNetwork.addEdge(Edge(id1, id2, edge.m_length, edge.m_diameter, edge.m_supply));
		}
	}
}


void Network::writeNetworkCSV(const IBK::Path &file) const{
	std::ofstream f;
	f.open(file.str(), std::ofstream::out | std::ofstream::trunc);
	for (const Edge &e: m_edges){
		f.precision(10);
		f << std::fixed << e.m_node1->m_x << "\t" << e.m_node1->m_y << "\t" << e.m_node2->m_x << "\t"
		  << e.m_node2->m_y << "\t" << e.m_length << std::endl;
	}
	f.close();
}


void Network::writePathCSV(const IBK::Path &file, const Node & node, const std::vector<Edge *> &path) const {
	std::ofstream f;
	f.open(file.str(), std::ofstream::out | std::ofstream::trunc);
	f.precision(10);
	f << std::fixed << node.m_x << "\t" << node.m_y << std::endl;
	for (const Edge *e: path){
		f << std::fixed << e->m_node1->m_x << "\t" << e->m_node1->m_y << "\t" << e->m_node2->m_x << "\t"
		  << e->m_node2->m_y << "\t" << e->m_length << std::endl;
	}
	f.close();
}


void Network::writeBuildingsCSV(const IBK::Path &file) const {
	std::ofstream f;
	f.open(file.str(), std::ofstream::out | std::ofstream::trunc);
	f.precision(10);
	for (const Node &n: m_nodes){
		if (n.m_type==Node::NT_Building)
			f << std::fixed << n.m_x << "\t" << n.m_y << "\t" << n.m_heatingDemand << std::endl;
	}
	f.close();
}


void Network::dijkstraShortestPathToSource(Node &startNode, std::vector<Edge*> &pathSourceToStart){

	// init: all nodes have infinte distance to start node and no predecessor
	for (Node &n: m_nodes){
		n.m_distanceToStart = std::numeric_limits<double>::max();
		n.m_predecessor = nullptr;
	}
	startNode.m_distanceToStart = 0;
	std::set<unsigned> visitedNodes;

	// go through all not-visited nodes
	while (visitedNodes.size() <= m_nodes.size()){
		// find node with currently smallest distance to start, which has not yet been visited:
		double minDistance = std::numeric_limits<double>::max();
		Node *nMin = nullptr;
		for (unsigned id = 0; id < m_nodes.size(); ++id){
			if (visitedNodes.find(id) == visitedNodes.end() && m_nodes[id].m_distanceToStart < minDistance){
				minDistance = m_nodes[id].m_distanceToStart;
				nMin = &m_nodes[id];
			}
		}
		// if source reached: return path
		if (nMin->m_type == Node::NT_Source){
			nMin->pathToNull(pathSourceToStart);
			return;
		}
		// update distance from start to neighbours of nMin
		visitedNodes.insert(nMin->m_id);
		nMin->updateNeighbourDistances();
	}
}


double Network::pressureLossColebrook(const double &diameter, const double &length, const double &roughness,
									  const double &massFlow, const double &fluidDensity, const double &fluidKinViscosity){

	double velocity = massFlow / (fluidDensity * diameter * diameter * 3.14159 / 4);
	double Re = velocity * diameter / fluidKinViscosity;
	double lambda = 0.05;
	double lambda_new = lambda;
	for (unsigned n=0; n<100; ++n){
		lambda_new = std::pow(-2 * std::log10(2.51 / (Re * std::sqrt(lambda)) + roughness / (3.71 * diameter)), -2);
		if (abs(lambda_new - lambda) / lambda < 1e-3)
			break;
		lambda = lambda_new;
	}
	return lambda_new * length / diameter * fluidDensity / 2 * velocity * velocity;
}



} // namespace VICUS
