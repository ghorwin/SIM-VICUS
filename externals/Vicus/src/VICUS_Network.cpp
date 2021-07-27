/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

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


#define PI				3.141592653589793238

namespace VICUS {



// *** copy from NM_Physics ***

double ReynoldsNumber(const double &v, const double &kinVis, const double &d)
{
	return  v * d / kinVis;
}

/*! Reynolds number where flow switches from laminar to transition state. */
#define RE_LAMINAR		1700.0

/*! Reynolds number where flow switches from transition state to turbulent */
#define RE_TURBULENT	4000.0

double NusseltNumberTurbulent(const double &reynolds, const double &prandtl, const double &l, const double &d)
{
	IBK_ASSERT(reynolds>0);
	// Equation 25, VDI-Wärmeatlas (11. Auflage), Kapitel B2, Page 28
	double zeta = std::pow(1.8 * std::log10(reynolds) - 1.5, -2.0);
	return zeta / 8. * reynolds*prandtl /
		(1. + 12.7 * std::sqrt(zeta / 8.) * (std::pow(prandtl, 0.6667) - 1.)) *
							 (1. + std::pow(d / l, 0.6667));
}

double NusseltNumberLaminar(const double &reynolds, const double &prandtl, const double &l, const double &d)
{
	if (reynolds <=0)
		return 3.66; // for velocity=0
	else
		return std::pow( 49.37 + std::pow(1.615 * std::pow(reynolds * prandtl * d/l, 1.0/3.0) - 0.7, 3.0) , 1.0/3.0);
}

double FrictionFactorSwamee(const double &reynolds, const double &d, const double &roughness){
	if (reynolds < RE_LAMINAR)
		return 64.0/reynolds;
	else if (reynolds < RE_TURBULENT){
		double fLam = 64.0/RE_LAMINAR; // f(RE_LAMINAR)
		double fTurb = std::log10((roughness / d) / 3.7 + 5.74 / std::pow(RE_TURBULENT, 0.9) );
		fTurb = 0.25/(fTurb*fTurb); // f(RE_TURBULENT)
		// now interpolate linearly between fLam and fTurb
		return fLam + (reynolds - RE_LAMINAR) * (fTurb - fLam) / (RE_TURBULENT - RE_LAMINAR);
	}
	else{
		double f = std::log10( (roughness / d) / 3.7 + 5.74 / std::pow(reynolds, 0.9) ) ;
		return	0.25 / (f*f);
	}
}


double NusseltNumber(const double &reynolds, const double &prandtl, const double &l, const double &d)
{
	if (reynolds < RE_LAMINAR){
		return NusseltNumberLaminar(reynolds, prandtl, l, d);
	}
	else if (reynolds < RE_TURBULENT){
		double nuLam = NusseltNumberLaminar(RE_LAMINAR, prandtl, l, d);
		double nuTurb = NusseltNumberTurbulent(RE_TURBULENT, prandtl, l, d);
		return nuLam + (reynolds - RE_LAMINAR) * (nuTurb - nuLam) / (RE_TURBULENT - RE_LAMINAR);
	}
	else {
		return NusseltNumberTurbulent(reynolds, prandtl, l, d);
	}
}

double PrandtlNumber(const double &kinVis, const double &cp, const double &lambda, const double &rho)
{
	return kinVis * cp * rho / lambda;
}




Network::Network() {
	setDefaultSizingParams();
}


Network Network::copyWithBaseParameters() {
	Network copy;
	copy.m_type = this->m_type;
	copy.m_modelType = this->m_modelType;
	copy.m_fluidID = this->m_fluidID;
	copy.m_scaleEdges = this->m_scaleEdges;
	copy.m_scaleNodes = this->m_scaleNodes;
	copy.m_origin = this->m_origin;
	for (unsigned int i=0; i<VICUS::Network::NUM_P; ++i)
		copy.m_para[i] = this->m_para[i];

	return copy;
}


unsigned Network::addNode(const IBKMK::Vector3D &v, const NetworkNode::NodeType type, const bool consistentCoordinates) {

	// if there is an existing node with identical coordinates, return its id and dont add a new one
	if (consistentCoordinates){
		for (NetworkNode n: m_nodes){
			if (n.m_position.distanceTo(v) < geometricResolution)
				return n.m_id;
		}
	}

	// else add new node
	unsigned id = m_nodes.size();
	m_nodes.push_back(NetworkNode(id, type, v));
	updateNodeEdgeConnectionPointers();

	return id;
}


unsigned Network::addNode(const NetworkNode &node, const bool considerCoordinates) {
	unsigned id = addNode(node.m_position, node.m_type, considerCoordinates);
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

	// loop over all edges
	for (NetworkEdge & e : m_edges) {
		// store pointers to connected nodes
		IBK_ASSERT(e.nodeId1() < m_nodes.size());
		e.m_node1 = &m_nodes[e.nodeId1()];
		IBK_ASSERT(e.nodeId2() < m_nodes.size());
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


void Network::updateVisualizationRadius(const VICUS::Database<VICUS::NetworkPipe> & pipeDB) const{

	// process all edges and update their display radius
	for (const VICUS::NetworkEdge & e : m_edges) {
		double radius = 0.5;
		if (e.m_pipeId != VICUS::INVALID_ID){
			const VICUS::NetworkPipe * pipe = pipeDB[e.m_pipeId];
			if (pipe != nullptr)
				radius *= pipe->m_para[VICUS::NetworkPipe::P_DiameterOutside].value * m_scaleEdges;
		}
		e.m_visualizationRadius = radius;
	}

	// process all nodes and update their display radius, default radius = 1 cm
	for (const VICUS::NetworkNode & no : m_nodes) {
		double radius = 1 * m_scaleNodes / 100;
		switch (no.m_type) {
			case NetworkNode::NT_Building: {
				// scale node by heating demand - 1 mm / 1000 W; 4800 W -> 48 * 0.01 = radius = 0.48
				if (no.m_maxHeatingDemand.value > 0)
					radius *= no.m_maxHeatingDemand.value / 1000;
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


void Network::setDefaultColors() const
{
	for (const NetworkEdge & edge: m_edges)
		edge.m_color = Qt::lightGray;
	for (const NetworkNode & node: m_nodes) {
		switch (node.m_type) {
			case VICUS::NetworkNode::NT_Source:
				node.m_color = Qt::darkGray;
			break;
			case VICUS::NetworkNode::NT_Building:
				node.m_color = Qt::darkGray;
			break;
			case VICUS::NetworkNode::NT_Mixer:
				node.m_color = Qt::lightGray;
			break;
			default:;
		}
	}
}


void Network::setVisible(bool visible)
{
	m_visible = visible;
	for (NetworkEdge &edge: m_edges)
		edge.m_visible = visible;
	for (NetworkNode &node: m_nodes)
		node.m_visible = visible;
}


QColor Network::colorHeatExchangeType(NANDRAD::HydraulicNetworkHeatExchange::ModelType heatExchangeType) {
	switch (heatExchangeType) {
		case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSpline:
			return QColor("#8E1517");
		case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureZone:
			return QColor("#F82529");
		case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossConstant:
			return QColor("#F3722C");
		case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSpline:
			return QColor("#F8961E");
		case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstant:
			return QColor("#F9C74F");
		case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSplineCondenser:
			return QColor("#364959");
		case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSplineEvaporator:
			return QColor("#90BE6D");
		case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstructionLayer:
			return QColor("#34836c");
		case NANDRAD::HydraulicNetworkHeatExchange::NUM_T:
			return QColor("#5B4869");
	}
	return Qt::red;
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
		m_nodes[id].m_maxHeatingDemand = IBK::Parameter("MaxHeatingDemand", heatDemand, "W");
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
			NetworkLine l1 = NetworkLine(m_edges[i1]);
			NetworkLine l2 = NetworkLine(m_edges[i2]);
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
			double dist = NetworkLine(m_edges[id]).distanceToPoint(m_nodes[idBuilding].m_position.point2D());
			if (dist<distMin){
				distMin = dist;
				idEdgeMin = id;
			}
		}
		// branch node
		NetworkLine lMin = NetworkLine(m_edges[idEdgeMin]);
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
			double dist1 = NetworkLine::distanceBetweenPoints(pBranch, m_edges[idEdgeMin].m_node1->m_position.point2D());
			double dist2 = NetworkLine::distanceBetweenPoints(pBranch, m_edges[idEdgeMin].m_node2->m_position.point2D());
			idBranch = (dist1 < dist2) ? m_edges[idEdgeMin].nodeId1() : m_edges[idEdgeMin].nodeId2();
			// if pipe should be extended, change coordinates of branch node
			if (extendSupplyPipes) {
				m_nodes[idBranch].m_position = IBKMK::Vector3D(pBranch);
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
			return (int)nBuilding.m_id;
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


void Network::removeShortEdges(Network &newNetwork, const double &threshold) {
	FUNCID(Network::removeShortEdges);

	updateNodeEdgeConnectionPointers();

	// check for source
	std::vector<NetworkNode> sources;
	findSourceNodes(sources);
	if (sources.size() < 1)
		throw IBK::Exception("Network has no source node. Set one node to type source.", FUNC_ID);

	// First, we need to order the edges. As a result we get a list of edges where each edge
	// is connected to one of the previuos edges in the vector.
	std::set<const VICUS::NetworkNode *> dummyNodeSet;
	std::vector<const VICUS::NetworkEdge *> orderedEdges;
	for (const VICUS::NetworkNode &node: m_nodes){
		if (node.m_type == VICUS::NetworkNode::NT_Source){
			node.setInletOutletNode(dummyNodeSet, orderedEdges);
			break;
		}
	}

	// Now we create the new network:
	// If the edge length is above the threshold, we add it with the according nodes to the new network.
	// Else: We map the unknown node to the other node. There is always only one unknown node! This is because we ordered
	// the edges in advance, see above.

	// the very first node of the first edge is already known in the map (this must be the source)
	std::map<unsigned int, unsigned int> nodeMap;
	unsigned int id0 = newNetwork.addNode(*orderedEdges[0]->m_node1);
	nodeMap[id0] = id0;

	for (const NetworkEdge *edge: orderedEdges){

		// get the ids of the nodes (they might have changed, when an edge was removed)
		unsigned int newId, existingId;
		if (contains(nodeMap, edge->nodeId1())){
			newId = edge->nodeId2();
			existingId = nodeMap.at(edge->nodeId1());
		}
		else if (contains(nodeMap, edge->nodeId2())){
			newId = edge->nodeId1();
			existingId = nodeMap.at(edge->nodeId2());
		}
		else
			throw IBK::Exception(IBK::FormatString("Error in edge '%1 -> %2': One of both nodes must exist already in the map.")
													.arg(edge->nodeId1()).arg(edge->nodeId2()), FUNC_ID);

//		// calculate the updated length
		double length0 = edge->length();
		double length = NetworkLine(m_nodes[newId].m_position.point2D(), m_nodes[existingId].m_position.point2D()).length();

		// if this length is below the threshold and none of the nodes is a building,
		// we will just map the ids and dont add any edge
		if (length < threshold && (edge->m_node1->m_type != NetworkNode::NT_Building &&
								   edge->m_node2->m_type != NetworkNode::NT_Building) ){

			// find the node which is not known yet
			// map this node to the mapped value of the known node
			nodeMap[newId] = nodeMap.at(existingId);
		}

		// else just add the nodes and the edge
		else {

			NetworkNode * newNode;
			if (newId == edge->nodeId1())
				newNode = edge->m_node1;
			else
				newNode = edge->m_node2;

			unsigned int id33 = newNetwork.addNode(*newNode);
			nodeMap[newId] = newId;

			// add edge and calculate new length of it
			newNetwork.addEdge(NetworkEdge(existingId, id33, edge->m_supply, edge->length(), edge->m_pipeId));
			newNetwork.edge(existingId, id33)->setLengthFromCoordinates();
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
	for (unsigned int n = 0; n < NUM_P; ++n){
		try {
			m_para[VICUS::Network::P_MaxPressureLoss].checkedValue("MaxPressureLoss", "Pa/m", "Pa/m",
																   std::numeric_limits<double>::lowest(), true,
																   std::numeric_limits<double>::max(), true, nullptr);
			m_para[VICUS::Network::P_TemperatureSetpoint].checkedValue("TemperatureSetpoint", "C", "C",
																   std::numeric_limits<double>::lowest(), true,
																   std::numeric_limits<double>::max(), true, nullptr);
			m_para[VICUS::Network::P_TemperatureDifference].checkedValue("TemperatureDifference", "K", "K",
																   std::numeric_limits<double>::lowest(), true,
																   std::numeric_limits<double>::max(), true, nullptr);
		} catch (IBK::Exception &ex) {
			throw IBK::Exception(ex, "Error in sizing pipes algorithm!", FUNC_ID);
		}
	}

	// check for source
	std::vector<NetworkNode> sources;
	findSourceNodes(sources);
	if (sources.size() < 1)
		throw IBK::Exception("Network has no source node. Set one node to type source.", FUNC_ID);

	// set all edges heating demand = 0
	for (NetworkEdge &edge: m_edges)
		edge.m_nominalHeatingDemand = 0;

	// for all buildings: add their heating demand to the pipes along their shortest path
	for (NetworkNode &node: m_nodes) {
		if (node.m_type != NetworkNode::NT_Building)
			continue;
		if (node.m_maxHeatingDemand.value <= 0)
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
			edge->m_nominalHeatingDemand += node.m_maxHeatingDemand.value;
	}

	// in case there is a pipe which is not part of any path (e.g. in circular grid): assign the adjacent heating demand
	for (NetworkEdge &e: m_edges){
		if (e.m_nominalHeatingDemand <= 0){
			std::set<NetworkEdge *> edges1, edges2;
			e.m_nominalHeatingDemand = 0.5 * ( e.m_node1->adjacentHeatingDemand(edges1)
										+ e.m_node2->adjacentHeatingDemand(edges2) );
		}
	}

	// determine pipe with largest inner diameter
	const NetworkPipe * largestPipe = availablePipes[0];
	for (const NetworkPipe * pipe: availablePipes) {
		if (pipe->diameterInside() > largestPipe->diameterInside())
			largestPipe = pipe;
	}

	double rho = fluid->m_para[NetworkFluid::P_Density].value;
	double kinvis = fluid->m_kinematicViscosity.m_values.value(m_para[P_TemperatureSetpoint].get_value("C"));
	double cp = fluid->m_para[NetworkFluid::P_HeatCapacity].value;

	// for each edge: find the smallest pipe from DB that has a pressure loss below deltapMax
	double deltaPMax = m_para[P_MaxPressureLoss].get_value("Pa/m");
	for (NetworkEdge &e: m_edges) {
		const NetworkPipe * currentPipe = nullptr; // here we store the candidate for the pipe to use

		for (const NetworkPipe * pipe : availablePipes){
			e.m_nominalMassFlow = e.m_nominalHeatingDemand / (m_para[P_TemperatureDifference].get_value("K") * cp);
			double di = pipe->diameterInside();
			double area = PI/4 * di * di;
			double vel = e.m_nominalMassFlow / (rho * area);
			double re = ReynoldsNumber(vel, kinvis, di);
			//  pressure loss per length (Pa/m)
			double zeta = 1.0 / di * FrictionFactorSwamee(re, di, m_para[VICUS::NetworkPipe::P_DiameterOutside].value);
			double dp = zeta * rho/2 * vel * vel;
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



void Network::calcTemperatureChangeIndicator(const NetworkFluid *fluid, const Database<NetworkPipe> &pipeDB) {
	FUNCID(Network::calcTemperatureChangeIndicator);

	// check parameters
	for (unsigned int n = 0; n < NUM_P; ++n){
		try {
			m_para[VICUS::Network::P_MaxPressureLoss].checkedValue("MaxPressureLoss", "Pa/m", "Pa/m",
																   std::numeric_limits<double>::lowest(), true,
																   std::numeric_limits<double>::max(), true, nullptr);
			m_para[VICUS::Network::P_TemperatureSetpoint].checkedValue("TemperatureSetpoint", "C", "C",
																   std::numeric_limits<double>::lowest(), true,
																   std::numeric_limits<double>::max(), true, nullptr);
			m_para[VICUS::Network::P_TemperatureDifference].checkedValue("TemperatureDifference", "K", "K",
																   std::numeric_limits<double>::lowest(), true,
																   std::numeric_limits<double>::max(), true, nullptr);
		} catch (IBK::Exception &ex) {
			throw IBK::Exception(ex, "Error in sizing pipes algorithm!", FUNC_ID);
		}
	}

	// check for source
	std::vector<NetworkNode> sources;
	findSourceNodes(sources);
	if (sources.size() < 1)
		throw IBK::Exception("Network has no source node. Set one node to type source.", FUNC_ID);

	// set all edges heating demand = 0
	for (NetworkEdge &edge: m_edges)
		edge.m_nominalHeatingDemand = 0;

	// for all buildings: add their heating demand to the pipes along their shortest path
	for (NetworkNode &node: m_nodes) {
		if (node.m_type != NetworkNode::NT_Building)
			continue;
		if (node.m_maxHeatingDemand.value <= 0)
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
			edge->m_nominalHeatingDemand += node.m_maxHeatingDemand.value;
	}

	// in case there is a pipe which is not part of any path (e.g. in circular grid): assign the adjacent heating demand
	for (NetworkEdge &e: m_edges){
		if (e.m_nominalHeatingDemand <= 0){
			std::set<NetworkEdge *> edges1, edges2;
			e.m_nominalHeatingDemand = 0.5 * ( e.m_node1->adjacentHeatingDemand(edges1)
										+ e.m_node2->adjacentHeatingDemand(edges2) );
		}
	}


	double rho = fluid->m_para[NetworkFluid::P_Density].value;
	double kinvis = fluid->m_kinematicViscosity.m_values.value(m_para[P_TemperatureSetpoint].get_value("C"));
	double cp = fluid->m_para[NetworkFluid::P_HeatCapacity].value;
	double lambda = fluid->m_para[NetworkFluid::P_Conductivity].value;

	for (NetworkEdge &e: m_edges){

		const NetworkPipe *pipe = pipeDB[e.m_pipeId];
		Q_ASSERT(pipe != nullptr);
		e.m_nominalMassFlow = e.m_nominalHeatingDemand / (m_para[P_TemperatureDifference].get_value("K") * cp);

		double di = pipe->diameterInside();
		double area = PI/4 * di * di;
		double vel = e.m_nominalMassFlow / (rho * area);
		double re = ReynoldsNumber(vel, kinvis, di);
		double prandtl = PrandtlNumber(kinvis, cp, lambda, rho);
		double nusselt = NusseltNumber(re, prandtl, e.length(), pipe->diameterInside());
		double innerHeatTransferCoefficient = nusselt * lambda / pipe->diameterInside();
		double UAValue = e.length() /
				(
					  1.0 / ( innerHeatTransferCoefficient * pipe->diameterInside() * PI)
					+ 1.0 / pipe->UValue()
				);

		e.m_tempChangeIndicator = UAValue / (e.m_nominalMassFlow * cp);
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


size_t Network::numberOfBuildings() const{
	size_t count = 0;
	for (const NetworkNode &n: m_nodes){
		if (n.m_type == NetworkNode::NT_Building)
			++count;
	}
	return count;
}


void Network::setDefaultSizingParams() {
	m_para[Network::para_t::P_TemperatureSetpoint] = IBK::Parameter("TemperatureSetpoint", 5, IBK::Unit("C"));
	KeywordList::setParameter(m_para, "Network::para_t", Network::para_t::P_TemperatureDifference, 5);
	KeywordList::setParameter(m_para, "Network::para_t", Network::para_t::P_MaxPressureLoss, 150);
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
			f << std::fixed << n.m_position.m_x << "\t" << n.m_position.m_y << "\t" << n.m_maxHeatingDemand.value << std::endl;
	}
	f.close();
}


} // namespace VICUS
