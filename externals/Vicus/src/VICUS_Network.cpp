#include "VICUS_Network.h"

#include <IBK_assert.h>
#include <IBK_Path.h>

#include <fstream>
#include <algorithm>

namespace VICUS {

Network::Network() {

}


unsigned Network::addNode(const double &x, const double &y, const Network::Node::NodeType type, const bool consistentCoordinates)
{
	// if there is an existing node with identical coordinates, return its id and dont add a new one
	if (consistentCoordinates){
		for (Node n: m_nodes){
			if (Line::pointsMatch(n.m_x, n.m_y, x, y)) // threshold 1 cm
				return n.m_id;
		}
	}
	unsigned id = m_nodes.size();
	m_nodes.push_back(Node(id, x, y, type));
	updateNodeEdgeConnectionPointers();

	return id;
}

unsigned Network::addNode(const Network::Node &node, const bool considerCoordinates)
{
	return addNode(node.m_x, node.m_y, node.m_type, considerCoordinates);
}


void Network::addEdge(const unsigned nodeId1, const unsigned nodeId2, const bool supply)
{
	IBK_ASSERT(nodeId1<m_nodes.size() && nodeId2<m_nodes.size());
	m_edges.push_back(Edge(nodeId1, nodeId2, supply));
	updateNodeEdgeConnectionPointers();
}

void Network::addEdge(const Network::Edge &edge)
{
	IBK_ASSERT(edge.m_nodeId1<m_nodes.size() && edge.m_nodeId2<m_nodes.size());
	m_edges.push_back(edge);
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


void Network::Edge::collectConnectedNodes(std::set<const Network::Node *> & connectedNodes,
										  std::set<const Network::Edge *> & connectedEdge) const {
	// first store ourselves as connected
	connectedEdge.insert(this);
	// now ask our nodes to collect their connected elements
	m_node1->collectConnectedEdges(connectedNodes, connectedEdge);
	m_node2->collectConnectedEdges(connectedNodes, connectedEdge);
}


Network::Node * Network::Edge::neighbourNode(const Network::Node *node) const{
	IBK_ASSERT(node->m_id == m_nodeId1 || node->m_id == m_nodeId2);
	if (node->m_id == m_nodeId1)
		return m_node2;
	else
		return m_node1;
}


void Network::Node::collectConnectedEdges(std::set<const Network::Node *> & connectedNodes,
										  std::set<const Network::Edge *> & connectedEdge) const {
	// store ourselves as connected
	connectedNodes.insert(this);
	// now ask connected elements to collect their nodes
	for (const Edge * e : m_edges) {
		// only process edges that are not yet collected
		if (connectedEdge.find(e) == connectedEdge.end())
			e->collectConnectedNodes(connectedNodes, connectedEdge);
	}
}


Network::Edge *Network::Node::neighborEdge(const Network::Edge *e) const
{
	IBK_ASSERT(m_edges.size()==2);
	if (m_edges[0] == e)
		return m_edges[1];
	else
		return m_edges[0];
}


void Network::readCSV(const IBK::Path &filePath, std::vector<std::string> &content) {
	FUNCID(Network::readCSV);

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

	std::vector<std::string> cont;
	readCSV(filePath, cont);

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
			polyLine.push_back(std::vector<double> {IBK::string2val<double>(xyStr[0]), IBK::string2val<double>(xyStr[1])});
		}
		for (unsigned i=0; i<polyLine.size()-1; ++i){
			unsigned n1 = addNode(polyLine[i][0], polyLine[i][1], Node::NT_Mixer);
			unsigned n2 = addNode(polyLine[i+1][0], polyLine[i+1][1], Node::NT_Mixer);
			addEdge(n1, n2, true);
		}
	}
}

void Network::readBuildingsFromCSV(const IBK::Path &filePath, const double &heatDemand)
{
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
		unsigned id = addNode(IBK::string2val<double>(xyStr[0]), IBK::string2val<double>(xyStr[1]), Node::NT_Building, heatDemand);
	}
}


void Network::setSource(const double &x, const double &y){
	Node * nMin = nullptr;
	double distMin = std::numeric_limits<double>::max();
	for (Node &n: m_nodes){
		double dist = Network::Line::distanceBetweenPoints(x, y, n.m_x, n.m_y);
		if (dist < distMin){
			distMin = dist;
			nMin = &n;
		}
	}
	nMin->m_type = Network::Node::NT_Source;
}


void Network::generateIntersections(){
	while (findIntersection()){
	}
}


bool Network::findIntersection() {
	for (unsigned i1=0; i1<m_edges.size(); ++i1) {
		for (unsigned i2=i1+1; i2<m_edges.size(); ++i2) {
			// calculate intersection and check if it is within both lines
			Line l1 = Line(m_edges[i1]);
			Line l2 = Line(m_edges[i2]);
			double xs, ys;
			l1.intersection(l2, xs, ys);
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

// THIS ONE HAS STORAGE PROBLEMS ???
//void Network::connectBuildings(const bool extendSupplyPipes) {

//	int idNext = nextUnconnectedBuilding();
//	Edge *eMin = nullptr;
//	while (idNext>=0) {

//		unsigned idBuilding = static_cast<unsigned>(idNext);

//		// find closest supply edge
//		double distMin = std::numeric_limits<double>::max();
//		for (Edge & e: m_edges){
//			if (!e.m_supply)
//				continue;
//			double dist = Line(e).distanceToPoint(m_nodes[idBuilding].m_x, m_nodes[idBuilding].m_y);
//			if (dist<distMin){
//				distMin = dist;
//				eMin = &e;
//			}
//		}
//		// branch node
//		Line lMin = Line(*eMin);
//		double xBranch, yBranch;
//		unsigned idBranch;
//		lMin.projectionFromPoint(m_nodes[idBuilding].m_x, m_nodes[idBuilding].m_y, xBranch, yBranch);
//		// branch node is inside edge: split edge
//		if (lMin.containsPoint(xBranch,yBranch)){
//			idBranch = addNode(xBranch, yBranch, Node::NT_Mixer);
//			addEdge(eMin->m_nodeId1, idBranch, true);
//			eMin->m_nodeId1 = idBranch;
//			updateNodeEdgeConnectionPointers();
//		} // branch node is outside edge
//		else{
//			double dist1 = Line::distanceBetweenPoints(xBranch, yBranch, eMin->m_node1->m_x, eMin->m_node1->m_y);
//			double dist2 = Line::distanceBetweenPoints(xBranch, yBranch, eMin->m_node2->m_x, eMin->m_node2->m_y);
//			idBranch = (dist1 < dist2) ? eMin->m_nodeId1 : eMin->m_nodeId2 ;
//			// if pipe should be extended, change coordinates of branch node
//			if (extendSupplyPipes){
//				m_nodes[idBranch].m_x = xBranch;
//				m_nodes[idBranch].m_y = yBranch;
//				updateNodeEdgeConnectionPointers();
//			}
//		}
//		// connect building to branch node
//		addEdge(idBranch, idBuilding, false);

//		idNext = nextUnconnectedBuilding();
//	}
//}


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
		} // branch node is outside edge
		else{
			double dist1 = Line::distanceBetweenPoints(xBranch, yBranch, m_edges[idEdgeMin].m_node1->m_x, m_edges[idEdgeMin].m_node1->m_y);
			double dist2 = Line::distanceBetweenPoints(xBranch, yBranch, m_edges[idEdgeMin].m_node2->m_x, m_edges[idEdgeMin].m_node2->m_y);
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


void Network::networkWithoutDeadEnds(Network &cleanNetwork) const{
	for (const Edge &e: m_edges){
		if (e.m_node1->isDeadEnd() || e.m_node2->isDeadEnd())
			continue;
		unsigned id1 = cleanNetwork.addNode(*e.m_node1);
		unsigned id2 = cleanNetwork.addNode(*e.m_node2);
		cleanNetwork.addEdge(Edge(id1, id2, e.m_length, e.m_diameter, e.m_supply));
	}
}


void Network::calculateLengths()
{
	for (Edge &e: m_edges){
		double l = Line(e).length();
		e.m_length = Line(e).length();
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


const Network::Node * Network::Node::findNextNonRedundantNode(std::set<unsigned> & redundantNodes, double & totalLength, const Network::Edge *edgeToVisit) const
{
	totalLength += edgeToVisit->m_length;
	Node * nextNode = edgeToVisit->neighbourNode(this);
	if (!nextNode->isRedundant())
		return nextNode;
	redundantNodes.insert(nextNode->m_id);
	return nextNode->findNextNonRedundantNode(redundantNodes, totalLength, nextNode->neighborEdge(edgeToVisit));
}


bool Network::Node::findPathToSource(std::set<Edge*> &path, std::set<Network::Edge*> &visitedEdges, std::set<unsigned> &visitedNodes){

	if (visitedNodes.find(m_id) != visitedNodes.end())
		return false;
	visitedNodes.insert(m_id);

	// if reached end of graph
	if (m_edges.size()==1 && visitedEdges.find(m_edges[0]) != visitedEdges.end()){
		if (m_type==NodeType::NT_Source){
			return true;
		}
		return false;
	}

	for (Edge *e: m_edges){
		if (visitedEdges.find(e) == visitedEdges.end()){
			visitedEdges.insert(e);
			if (e->neighbourNode(this)->findPathToSource(path, visitedEdges, visitedNodes)){
				path.insert(e);
				return true;
			}
		}
	}
	return false;
}


void Network::Node::updateNeighbourDistances() {

	for (Edge *e :m_edges){
		// calculate alternative distance to neighbour. If it is shorter than current one:  set it as its current distance
		Node * neighbour = e->neighbourNode(this);
		double alternativeDistance = m_distanceToStart + e->m_length;
		if (alternativeDistance < neighbour->m_distanceToStart){
			neighbour->m_distanceToStart = alternativeDistance;
			neighbour->m_predecessor = this;
		}
	}
}


void Network::Node::getPathToNull(std::vector<Network::Edge *> &path){
	if (m_predecessor == nullptr)
		return;
	for (Edge * e: m_edges){
		if (e->neighbourNode(this) == m_predecessor){
			path.push_back(e);
			e->neighbourNode(this)->getPathToNull(path);
		}
	}
}


void Network::writeNetworkCSV(const IBK::Path &file) const{
	std::ofstream f;
	f.open(file.str());
	for (const Edge &e: m_edges){
		f.precision(10);
		f << std::fixed << e.m_node1->m_x << "\t" << e.m_node1->m_y << "\t" << e.m_node2->m_x << "\t" << e.m_node2->m_y << "\t" << e.m_length << std::endl;
	}
	f.close();
}


void Network::writePathCSV(const IBK::Path &file, const VICUS::Network::Node & node, const std::vector<VICUS::Network::Edge*> &path) const {
	std::ofstream f;
	f.open(file.str());
	f.precision(10);
	f << std::fixed << node.m_x << "\t" << node.m_y << std::endl;
	for (const Edge *e: path){
		f << std::fixed << e->m_node1->m_x << "\t" << e->m_node1->m_y << "\t" << e->m_node2->m_x << "\t" << e->m_node2->m_y << "\t" << e->m_length << std::endl;
	}
	f.close();
}


void Network::writeBuildingsCSV(const IBK::Path &file) const {
	std::ofstream f;
	f.open(file.str());
	f.precision(10);
	for (const Node &n: m_nodes){
		if (n.m_type==Network::Node::NT_Building)
			f << std::fixed << n.m_x << "\t" << n.m_y << "\t" << n.m_heatDemand << std::endl;
	}
	f.close();
}


void Network::dijkstraShortestPath(Node &startNode, std::vector<Edge*> &pathSourceToStart){

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
			nMin->getPathToNull(pathSourceToStart);
			return;
		}
		// update distance from start to neighbours of nMin
		visitedNodes.insert(nMin->m_id);
		nMin->updateNeighbourDistances();
	}
}


//void Network::Node::findRedundantNodes(std::set<unsigned> & redundantNodes, std::set<const Network::Edge *> & visitedEdges) const
//{
//	// redundant nodes have exactly 2 connected edges
//	if (this->isRedundant()){
//		redundantNodes.insert(this->m_id);
//	}
//	for (const Edge * e: m_edges){
//		// remember visited edges and dont visit them again
//		if (visitedEdges.find(e) == visitedEdges.end()){
//			visitedEdges.insert(e);
//			if (e->m_nodeId1 == this->m_id)
//				e->m_node2->findRedundantNodes(redundantNodes, visitedEdges);
//			else
//				e->m_node1->findRedundantNodes(redundantNodes, visitedEdges);
//		}

//	}
//}



void Network::Line::intersection(const Network::Line &line, double &xs, double &ys) const
{
	double x1 = m_x1; double x2 = m_x2; double x3 = line.m_x1; double x4 = line.m_x2;
	double y1 = m_y1; double y2 = m_y2; double y3 = line.m_y1; double y4 = line.m_y2;
	xs = ( (x4 - x3) * (x2 * y1 - x1 * y2) - (x2 - x1) * (x4 * y3 - x3 * y4) ) / ( (y4 - y3) * (x2 - x1) - (y2 - y1) * (x4 - x3) );
	ys = ( (y1 - y2) * (x4 * y3 - x3 * y4) - (y3 - y4) * (x2 * y1 - x1 * y2) ) / ( (y4 - y3) * (x2 - x1) - (y2 - y1) * (x4 - x3) );
}


void Network::Line::projectionFromPoint(const double &xp, const double &yp, double &xproj, double &yproj) const{
	// vector form g = a + s*b;  with a = (m_x1, m_y1)
	double b1 = m_x2 - m_x1;
	double b2 = m_y2 - m_y1;
	double s = (xp + b2/b1*yp - m_x1 - b2/b1*m_y1) / (b1 + b2*b2/b1);
	xproj = m_x1 + s*b1;
	yproj = m_y1 + s*b2;
}


double Network::Line::distanceToPoint(const double &xp, const double &yp) const{
	double xproj, yproj;
	projectionFromPoint(xp, yp, xproj, yproj);
	if (containsPoint(xproj, yproj))
		return distanceBetweenPoints(xp, yp, xproj, yproj);
	else
		return std::min(distanceBetweenPoints(m_x1, m_y1, xp, yp), distanceBetweenPoints(m_x2, m_y2, xp, yp));
}


std::pair<double, double> Network::Line::linearEquation() const
{
	double m = (m_y2 - m_y1) / (m_x2 - m_x1);
	double n = (m_y1*m_x2 - m_y2*m_x1) / (m_x2 - m_x1);
	return {m, n};
}

bool Network::Line::containsPoint(const double &xp, const double &yp) const
{
	bool inside = (xp >= std::min(m_x1, m_x2)) && (xp <= std::max(m_x1, m_x2)) && (yp >= std::min(m_y1, m_y2)) && (yp <= std::max(m_y1, m_y2));
	bool identity = pointsMatch(xp, yp, m_x1, m_y1) || pointsMatch(xp, yp, m_x2, m_y2);
	return inside && !identity;
}

bool Network::Line::sharesIntersection(const Network::Line &line) const
{
	double xp, yp;
	intersection(line, xp, yp);
	return containsPoint(xp, yp) && line.containsPoint(xp, yp);
}

double Network::Line::length() const
{
	return distanceBetweenPoints(m_x1, m_y1, m_x2, m_y2);
}

double Network::Line::distanceBetweenPoints(const double &x1, const double &y1, const double &x2, const double &y2)
{
	return std::sqrt( std::pow(x1-x2, 2) + std::pow(y1-y2, 2) );
}

bool Network::Line::pointsMatch(const double &x1, const double &y1, const double &x2, const double &y2, const double threshold)
{
	return distanceBetweenPoints(x1, y1, x2, y2) < threshold;
}




} // namespace VICUS
