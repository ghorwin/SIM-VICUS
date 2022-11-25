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
#include "VICUS_utilities.h"

#include <QFile>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>

#include <IBK_assert.h>
#include <IBK_Path.h>
#include <IBK_FileReader.h>
#include <IBK_FluidPhysics.h>

#include <IBKMK_3DCalculations.h>
//#include <IBKMK_UTM.h>




#include <fstream>
#include <algorithm>



#define PI				3.141592653589793238

namespace VICUS {



Network::Network() {

	// set default parameters for pipe sizing
	KeywordList::setParameter(m_para, "Network::para_t", Network::para_t::P_TemperatureSetpoint, 20);
	KeywordList::setParameter(m_para, "Network::para_t", Network::para_t::P_TemperatureDifference, 4);
	KeywordList::setParameter(m_para, "Network::para_t", Network::para_t::P_MaxPressureLoss, 150);

	// other default parameters
	KeywordList::setParameter(m_para, "Network::para_t", Network::para_t::P_MaxPipeDiscretization, 5);
	KeywordList::setParameter(m_para, "Network::para_t", Network::para_t::P_DefaultFluidTemperature, 20);
	KeywordList::setParameter(m_para, "Network::para_t", Network::para_t::P_InitialFluidTemperature, 20);
	KeywordList::setParameter(m_para, "Network::para_t", Network::para_t::P_ReferencePressure, 0);

}


Network Network::copyWithBaseParameters(unsigned int newID) {
	Network n;
	n.m_id = newID;
	n.m_type = this->m_type;
	n.m_modelType = this->m_modelType;
	n.m_idFluid = this->m_idFluid;
	n.m_scaleEdges = this->m_scaleEdges;
	n.m_scaleNodes = this->m_scaleNodes;
	n.m_origin = this->m_origin;
	for (unsigned int i=0; i<VICUS::Network::NUM_P; ++i)
		n.m_para[i] = this->m_para[i];

	return n;
}


unsigned int Network::addNode(unsigned int preferedId, const IBKMK::Vector3D &v, const NetworkNode::NodeType type, const bool consistentCoordinates) {

	// if there is an existing node with identical coordinates, return its id and dont add a new one
	if (consistentCoordinates){
		for (const NetworkNode &n: m_nodes){
			if (n.m_position.distanceTo(v) < GeometricResolution)
				return n.m_id;
		}
	}
	// else add new node
	m_nodes.push_back(NetworkNode(preferedId, type, v));
	updateNodeEdgeConnectionPointers();
	return preferedId;
}


unsigned int Network::addNode(unsigned int preferedId, const NetworkNode &node, const bool considerCoordinates) {
	unsigned id = addNode(preferedId, node.m_position, node.m_type, considerCoordinates);
	nodeById(id)->m_maxHeatingDemand = node.m_maxHeatingDemand;
	return id;
}


void Network::addEdge(const unsigned id, const unsigned nodeId1, const unsigned nodeId2, const bool supply, unsigned int pipePropId) {
	NetworkEdge e(id, nodeId1, nodeId2, supply, 0, pipePropId);
	m_edges.push_back(e);
	updateNodeEdgeConnectionPointers();
	m_edges.back().setLengthFromCoordinates();
}


void Network::addEdge(const NetworkEdge &edge) {
	m_edges.push_back(edge);
	// we DON'T recalculate the length here!
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
		e.m_node1 = nodeById(e.nodeId1());
		e.m_node2 = nodeById(e.nodeId2());

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


void Network::updateVisualizationRadius(const VICUS::Database<VICUS::NetworkPipe> & pipeDB) {

	// process all edges and update their display radius
	for (VICUS::NetworkEdge & e : m_edges) {
		double radius = 0.5;
		if (e.m_idPipe != VICUS::INVALID_ID){
			const VICUS::NetworkPipe * pipe = pipeDB[e.m_idPipe];
			if (pipe != nullptr)
				radius *= pipe->m_para[VICUS::NetworkPipe::P_DiameterOutside].value * m_scaleEdges;
		}
		e.m_visualizationRadius = radius;
	}

	// process all nodes and update their display radius, default radius = 1 cm
	for (const VICUS::NetworkNode & no : m_nodes) {
		double radius = 1 * m_scaleNodes / 100;
		switch (no.m_type) {
			case NetworkNode::NT_SubStation: {
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


void Network::setDefaultColors() const{
	for (const NetworkEdge & edge: m_edges)
		edge.m_color = Qt::lightGray;
	for (const NetworkNode & node: m_nodes) {
		switch (node.m_type) {
			case VICUS::NetworkNode::NT_Source:
				node.m_color = QColor(230, 138, 0); // orange
			break;
			case VICUS::NetworkNode::NT_SubStation:
				node.m_color = QColor(0, 107, 179); // blue
			break;
			case VICUS::NetworkNode::NT_Mixer:
				node.m_color = QColor(119, 179, 0); // green
			break;
			case VICUS::NetworkNode::NUM_NT:;
		}
	}
}


void Network::setVisible(bool visible) {
	m_visible = visible;
	for (NetworkEdge &edge: m_edges)
		edge.m_visible = visible;
	for (NetworkNode &node: m_nodes)
		node.m_visible = visible;
}


NetworkNode *Network::nodeById(unsigned int id) {
	for (NetworkNode &n: m_nodes){
		if (n.m_id == id)
			return &n;
	}
	IBK_ASSERT(false);
	return nullptr;
}


const NetworkNode *Network::nodeById(unsigned int id) const {
	for (const NetworkNode &n: m_nodes){
		if (n.m_id == id)
			return &n;
	}
	IBK_ASSERT(false);
	return nullptr;
}


unsigned int Network::indexOfNode(unsigned int id) const {
	for (unsigned int i=0; i<m_nodes.size(); ++i){
		if (m_nodes[i].m_id == id)
			return i;
	}
	IBK_ASSERT(false);
	return 99999;
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
			return QColor("#4cc9f0");
		case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstant:
			return QColor("#F9C74F");
		case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSplineCondenser:
			return QColor("#f72585");
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


void Network::readGridFromCSV(const IBK::Path &filePath, unsigned int nextId){
	std::vector<std::string> cont;
	IBK::FileReader::readAll(filePath, cont, std::vector<std::string>());

	// extract vector of string-xy-pairs
	std::vector<std::string> tokens;
	for (std::string &line: cont){
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
			unsigned n1 = addNode(++nextId, IBKMK::Vector3D(polyLine[i][0], polyLine[i][1], 0) - m_origin, NetworkNode::NT_Mixer);
			unsigned n2 = addNode(++nextId, IBKMK::Vector3D(polyLine[i+1][0], polyLine[i+1][1], 0) - m_origin, NetworkNode::NT_Mixer);
			addEdge(++nextId, n1, n2, true, INVALID_ID);
		}
	}
}

QJsonObject convert2QJsonObject(const IBK::Path &filePath) {
	QFile file;
	file.setFileName(filePath.absolutePath().c_str());
	if(!file.open(QIODevice::ReadOnly)){
		throw IBK::Exception("Json file couldn't be opened/found", "scr/VICUS_Network.cpp::convert2QJsonObject");
	}

	QByteArray byteArray;
	byteArray = file.readAll();
	file.close();


	QJsonParseError parseError;
	QJsonDocument jsonDoc;
	jsonDoc = QJsonDocument::fromJson(byteArray, &parseError);
	if(parseError.error != QJsonParseError::NoError){
		throw IBK::Exception(QString("Parse error at %1:%1").arg(parseError.offset).arg(parseError.errorString()).toStdString(), "scr/VICUS_Network.cpp::convert2QJsonObject");
	}

	return jsonDoc.object();
}


void Network::readGridFromGeoJson(const IBK::Path &filePath, unsigned int nextId, unsigned int defaultPipeId, const Database<NetworkPipe> &pipeDB) {

	QJsonObject jsonObj = convert2QJsonObject(filePath);

	QJsonArray features = jsonObj["features"].toArray();


	double referenceLon = features[0].toObject()["geometry"].toObject()["coordinates"].toArray()[0].toArray()[0].toDouble();
	double referenceLat = features[0].toObject()["geometry"].toObject()["coordinates"].toArray()[0].toArray()[1].toDouble();
	for(const QJsonValue & feature :  features){
		std::vector<std::vector<double> > polyLine;
		unsigned int pipeId = defaultPipeId;
		double bestDiff = 10000;

		QJsonObject properties = feature.toObject()["properties"].toObject();
		int thickness = properties["DN"].toDouble();

		//find the a fitting pipe from the available ones or keep default pipe
		for(unsigned int id : m_availablePipes){
			double pipeThickness = pipeDB[id]->m_para[NetworkPipe::P_DiameterOutside].get_value("mm");
			double diff = thickness - pipeThickness;
			if((diff) == 0){
				// if the pipe has exacly the same diameter take it
				pipeId = id;
				break;
			}
			if(diff * diff <= 1){
				//if the difference is less or equal than one cm, take it, but keep on looking for better ones
				if(bestDiff > (diff * diff)){
					bestDiff = (diff * diff);
					pipeId = id;
				}
			}
		}


		QJsonObject geometry = feature.toObject()["geometry"].toObject();
		if(geometry["type"].toString()  !=  "LineString")
			continue;
		for(const QJsonValue & coordinates : geometry["coordinates"].toArray()){
			double lon = coordinates.toArray()[0].toDouble();
			double lat = coordinates.toArray()[1].toDouble();
			double latMean = (referenceLat + lat) / 2 * 0.01745;
			double x = 111.3 * cos(latMean) * (referenceLon - lon) * 1000;
			double y = 111.3 * (referenceLat - lat) * 1000;
			polyLine.push_back({x, y});
		}


		for (unsigned i=0; i<polyLine.size()-1; ++i){
			unsigned n1 = addNode(++nextId, IBKMK::Vector3D(polyLine[i][0], polyLine[i][1], 0) - m_origin, NetworkNode::NT_Mixer);
			unsigned n2 = addNode(++nextId, IBKMK::Vector3D(polyLine[i+1][0], polyLine[i+1][1], 0) - m_origin, NetworkNode::NT_Mixer);
			addEdge(++nextId, n1, n2, true, pipeId);		}
	}
}


void Network::readBuildingsFromCSV(const IBK::Path &filePath, const double &heatDemand, unsigned int nextId) {
	std::vector<std::string> cont;
	IBK::FileReader::readAll(filePath, cont, std::vector<std::string>());

	// extract vector of string-xy
	std::vector<std::string> lineSepStr;
	std::vector<std::string> xyStr;
	for (std::string &line: cont){
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
		unsigned id = addNode(++nextId, IBKMK::Vector3D(IBK::string2val<double>(xyStr[0]), IBK::string2val<double>(xyStr[1]), 0) - m_origin,
				NetworkNode::NT_SubStation);
		nodeById(id)->m_maxHeatingDemand = IBK::Parameter("MaxHeatingDemand", heatDemand, "W");
	}
}

void Network::readBuildingsFromGeoJson(const IBK::Path &filePath, const double &heatDemand, unsigned int nextId) {
	QJsonObject jsonObj = convert2QJsonObject(filePath);

	QJsonArray features = jsonObj["features"].toArray();

	for(const QJsonValue & feature :  features){
		QJsonObject geometry = feature.toObject()["geometry"].toObject();
		if(geometry["type"].toString()  !=  "Point")
			continue;
		QJsonValue coordinates = geometry["coordinates"];

		double x = coordinates.toArray()[0].toDouble() * 10000; // scaling factor (maybe compute dynamically or asḱ user)
		double y = coordinates.toArray()[1].toDouble() * 10000;

		// add node
		unsigned id = addNode(++nextId, IBKMK::Vector3D(x, y, 0) - m_origin,
				NetworkNode::NT_SubStation);
		nodeById(id)->m_maxHeatingDemand = IBK::Parameter("MaxHeatingDemand", heatDemand, "W");
	}
}


void Network::generateIntersections(unsigned int nextUnusedId, std::vector<unsigned int> &addedNodes, std::vector<unsigned int> &addedEdges){

	bool foundIntersection = true;

	while (foundIntersection) {
		foundIntersection = false;
		for (unsigned i1=0; i1<m_edges.size(); ++i1) {
			for (unsigned i2=i1+1; i2<m_edges.size(); ++i2) {

				// calculate intersection
				NetworkLine l1 = NetworkLine(m_edges[i1]);
				NetworkLine l2 = NetworkLine(m_edges[i2]);
				IBKMK::Vector3D ps;
				l1.intersection(l2, ps);

				// if it is within both lines: add node and edges, adapt exisiting nodes
				if (l1.containsPoint(ps) && l2.containsPoint(ps)){
					unsigned nInter = addNode(++nextUnusedId, ps, NetworkNode::NT_Mixer);
					addedNodes.push_back(nInter);
					addEdge(++nextUnusedId, nInter, m_edges[i1].nodeId1(), true, m_edges[i1].m_idPipe);
					addedEdges.push_back(nextUnusedId);
					addEdge(++nextUnusedId, nInter, m_edges[i2].nodeId1(), true, m_edges[i2].m_idPipe);
					addedEdges.push_back(nextUnusedId);
					m_edges[i1].changeNode1(nodeById(nInter));
					m_edges[i2].changeNode1(nodeById(nInter));
					updateNodeEdgeConnectionPointers();
					// after we have found and add an intersection: leave both for loops and start again
					foundIntersection = true;
					continue;
				}
			}

			if (foundIntersection)
				continue;
		}
	}
}


void Network::connectBuildings(unsigned int nextUnusedId, const bool extendSupplyPipes) {

	int idNextBuilding = nextUnconnectedBuilding();

	while (idNextBuilding >= 0) {

		unsigned int idBuilding = (unsigned int)idNextBuilding;

		// find closest supply edge
		double distMin = std::numeric_limits<double>::max();
		unsigned int idxEdgeMin = std::numeric_limits<unsigned int>::max();
		for (unsigned idx=0; idx<m_edges.size(); ++idx){
			if (!m_edges[idx].m_supply)
				continue;
			double dist = NetworkLine(m_edges[idx]).distanceToPoint(nodeById(idBuilding)->m_position);
			if (dist<distMin){
				distMin = dist;
				idxEdgeMin = idx;
			}
		}

		// no supply edge found
		if (idxEdgeMin == std::numeric_limits<unsigned int>::max())
			return;

		// calculate branch node
		IBKMK::Vector3D pBranch;
		unsigned idBranch;
		double lineFactor = 0;
		IBKMK::Vector3D a1 = m_edges[idxEdgeMin].m_node1->m_position;
		IBKMK::Vector3D a2 = m_edges[idxEdgeMin].m_node2->m_position;
		IBKMK::Vector3D b = a2 - a1;
		IBKMK::lineToPointDistance(a1, b, nodeById(idBuilding)->m_position, lineFactor, pBranch);

		// check if branch point is "outside" edge, i.e. if it is not between both end points or less than 1 m away from them
		if (lineFactor * b.magnitude() < 1 || lineFactor * b.magnitude() > b.magnitude() - 1) {
			// now we look which end point of edge is closer to it
			double dist1 = (pBranch - a1).magnitude();
			double dist2 = (pBranch - a2).magnitude();
			idBranch = (dist1 < dist2) ? m_edges[idxEdgeMin].nodeId1() : m_edges[idxEdgeMin].nodeId2();
			// special case: the selected end point is not a mixer. We create a branch node 1 m away from this point
			if (nodeById(idBranch)->m_type != NetworkNode::NT_Mixer) {
				if (dist1 < dist2)
					pBranch = a1 + b * (1.0/b.magnitude());
				else
					pBranch = a2 - b * (1.0/b.magnitude());
				idBranch = addNode(++nextUnusedId, pBranch, NetworkNode::NT_Mixer);
				addEdge(++nextUnusedId, m_edges[idxEdgeMin].nodeId1(), idBranch, true, m_edges[idxEdgeMin].m_idPipe);
				m_edges[idxEdgeMin].changeNode1(nodeById(idBranch));
				updateNodeEdgeConnectionPointers();
			}
			// if pipe should be extended, change coordinates of branch node
			if (extendSupplyPipes) {
				nodeById(idBranch)->m_position = pBranch;
				for (NetworkEdge *e: nodeById(idBranch)->m_edges)
					e->setLengthFromCoordinates();
			}
		}

		// branch node is "inside" edge: split edge
		else {
			idBranch = addNode(++nextUnusedId, pBranch, NetworkNode::NT_Mixer);
			addEdge(++nextUnusedId, m_edges[idxEdgeMin].nodeId1(), idBranch, true, m_edges[idxEdgeMin].m_idPipe);
			m_edges[idxEdgeMin].changeNode1(nodeById(idBranch));
			updateNodeEdgeConnectionPointers();
		}

		// finally connect building to branch node
		addEdge(++nextUnusedId, idBranch, idBuilding, false, m_edges[idxEdgeMin].m_idPipe);

		idNextBuilding = nextUnconnectedBuilding();
	}
}


int Network::nextUnconnectedBuilding() const{
	for (const NetworkNode &nBuilding: m_nodes){
		if (nBuilding.m_type == NetworkNode::NT_SubStation && nBuilding.m_edges.size()==0)
			return (int)nBuilding.m_id;
	}
	return -1;
}


void Network::cleanDeadEnds(){
	for (unsigned i=0; i<m_nodes.size(); ++i){
		for (const NetworkNode &n: m_nodes){
			// look if this is a dead end
			nodeById(n.m_id)->updateIsDeadEnd();
			if (n.m_isDeadEnd){
				// if so, remove all edges and the node itself
				for (NetworkEdge *e: n.m_edges)
					m_edges.erase(m_edges.begin() + indexOfEdge(e->nodeId1(), e->nodeId2()) );
				m_nodes.erase(m_nodes.begin() + indexOfNode(n.m_id));
				updateNodeEdgeConnectionPointers();
				break;
			}
		}
	}
}


// TODO : no copy needed anymore with new data structure ?
void Network::cleanRedundantEdges(unsigned int nextUnusedId, Network & cleanNetwork) const{

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
			unsigned id1 = cleanNetwork.addNode(++nextUnusedId, *previousNode);
			unsigned id2 = cleanNetwork.addNode(++nextUnusedId, *nextNode);
			cleanNetwork.addEdge(NetworkEdge(++nextUnusedId, id1, id2, edge.m_supply, totalLength, edge.m_idPipe));
		}
		else{
			unsigned id1 = cleanNetwork.addNode(++nextUnusedId, *edge.m_node1);
			unsigned id2 = cleanNetwork.addNode(++nextUnusedId, *edge.m_node2);
			cleanNetwork.addEdge(NetworkEdge(++nextUnusedId, id1, id2, edge.m_supply, edge.length(), edge.m_idPipe));
		}
	}
}


void Network::removeShortEdges(const double &thresholdLength) {
	FUNCID(Network::removeShortEdges);

	updateNodeEdgeConnectionPointers();

	// check if there is a source node
	std::vector<NetworkNode> sources;
	findSourceNodes(sources);
	if (sources.size() < 1)
		throw IBK::Exception("Network has no source node. Set one node to type source.", FUNC_ID);


	// we iterate as many times as there are edges
	bool hasChanged = true;
	std::set<const VICUS::NetworkNode *> dummyNodeSet;
	std::vector<const VICUS::NetworkEdge *> orderedEdges;
	unsigned int size = m_edges.size();
	for (unsigned int count=0; count<size; ++count){

		// if the network has changed since last iteration:
		// we put edges in a deterministic order. They are ordered according to their distance from the source node
		// And even more important: We need to put them in an order so that one node of each edge has already occured in
		// one of the previuos edges within the vector
		if (hasChanged){
			orderedEdges.clear();
			dummyNodeSet.clear();
			nodeById(sources[0].m_id)->setInletOutletNode(dummyNodeSet, orderedEdges);
			hasChanged = false;
		}

		// we store the node ids that we have already processed
		std::set<unsigned int> processedIds;
		processedIds.insert(orderedEdges[0]->nodeId1());

		// now go through the ordered edges
		for (const NetworkEdge *e: orderedEdges){

			// determine which of both nodeIds has already been processed (=exId)
			// and which is of both is new (=newId)
			unsigned int newId;
			if (processedIds.find(e->nodeId1()) != processedIds.end())
				newId = e->nodeId2();
			else
				newId = e->nodeId1();
			processedIds.insert(newId);
			unsigned int exId = e->neighbourNode(newId);

			// if the length of this edge is below threshold and the new node is not a building: we want to
			// - modify all edges connected to the new node of this edge and connect them to the existing node
			// - remove the according edge and the new node
			// - leave the loop and start again so that we can assess the next edges with there adjusted lengths
			if (e->length() < thresholdLength && nodeById(newId)->m_type == NetworkNode::NT_Mixer){

				for (NetworkEdge *adjacentEdge: nodeById(newId)->m_edges){
					if (adjacentEdge->nodeId1() == newId)
						adjacentEdge->changeNode1(nodeById(exId));
					else
						adjacentEdge->changeNode2(nodeById(exId));
				}

				m_nodes.erase(m_nodes.begin() + indexOfNode(newId));
				m_edges.erase(m_edges.begin() + indexOfEdge(exId, exId));

				// now update the pointers and leave the loop, start again from the beginning
				updateNodeEdgeConnectionPointers();
				hasChanged = true;
				break;
			}
		}
	}

}


void Network::findShortestPathForBuildings(std::map<unsigned int, std::vector<NetworkEdge *> > &minPathMap) const{
	FUNCID(Network::findShortestPathForBuildings);

	// check for source
	std::vector<NetworkNode> sources;
	findSourceNodes(sources);
	if (sources.size() < 1)
		throw IBK::Exception("Network has no source node. Set one node to type source.", FUNC_ID);

	// iterate over all buildings
	minPathMap.clear();
	for (const NetworkNode &node: m_nodes) {

		if (node.m_type != NetworkNode::NT_SubStation)
			continue;

		// there must be a defined maximum heating demand
		if (node.m_maxHeatingDemand.value <= 0)
			throw IBK::Exception(IBK::FormatString("Maximum heating demand of node '%1' must be >0").arg(node.m_id), FUNC_ID);

		// for each source find the shortest path to current node. Finally select the shortest of these paths
		std::vector<NetworkEdge * > minPath;
		double minPathLength = std::numeric_limits<double>::max();
		for (const NetworkNode &source: sources){
			std::vector<NetworkEdge * > path;
			// shortest path between source and node
			dijkstraShortestPathToSource(node, source, path);
			double pathLength = 0;
			// length of this path
			for (NetworkEdge *edge: path)
				pathLength += edge->length();
			// minumum length of paths for each source to current building
			if (pathLength < minPathLength){
				minPathLength = pathLength;
				minPath = path;
			}
		}
		// finally store the shortest path
		minPathMap[node.m_id] = minPath;
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

	// set all edges heating demand = 0
	for (NetworkEdge &edge: m_edges)
		edge.m_nominalHeatingDemand = 0;

	// find shortest path for each building node to closest source node
	std::map<unsigned int, std::vector<NetworkEdge *> > shortestPaths;
	findShortestPathForBuildings(shortestPaths);

	// now for each building node: go along shortest path and add the nodes heating demand to each edge along that path
	for (auto it = shortestPaths.begin(); it != shortestPaths.end(); ++it){
		NetworkNode *building = nodeById(it->first);			// get pointer to building node
		std::vector<NetworkEdge *> &shortestPath = it->second; // for readability
		for (NetworkEdge * edge: shortestPath)
			edge->m_nominalHeatingDemand += building->m_maxHeatingDemand.value;
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
			double re = IBK::ReynoldsNumber(vel, kinvis, di);
			//  pressure loss per length (Pa/m)
			double zeta = 1.0 / di * IBK::FrictionFactorSwamee(re, di, m_para[VICUS::NetworkPipe::P_DiameterOutside].value);
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
			e.m_idPipe = currentPipe->m_id;
		// otherwise store ID of biggest pipe
		else
			e.m_idPipe = largestPipe->m_id;
	}

}


void Network::calcTemperatureChangeIndicator(const NetworkFluid & fluid, const Database<NetworkPipe> &pipeDB,
											 std::map<unsigned int, std::vector<NetworkEdge *> > &shortestPaths) const{
	FUNCID(Network::calcTemperatureChangeIndicator);

	// check for source
	std::vector<NetworkNode> sources;
	findSourceNodes(sources);
	if (sources.size() < 1)
		throw IBK::Exception("Network has no source node. Set one node to type source.", FUNC_ID);

	// set all edges heating demand = 0
	for (const NetworkEdge &edge: m_edges)
		const_cast<NetworkEdge&>(edge).m_nominalHeatingDemand = 0;

	// find shortest path for each building node to closest source node
	shortestPaths.clear();
	findShortestPathForBuildings(shortestPaths);

	// now for each building node: go along shortest path and add the nodes heating demand to each edge along that path
	for (auto it = shortestPaths.begin(); it != shortestPaths.end(); ++it){
		const NetworkNode *building = nodeById(it->first);			// get pointer to building node
		std::vector<NetworkEdge *> &shortestPath = it->second; // for readability
		for (NetworkEdge * edge: shortestPath)
			edge->m_nominalHeatingDemand += building->m_maxHeatingDemand.value;
	}

	// in case there is a pipe which is not part of any path (e.g. in circular grid): assign the adjacent heating demand
	for (const NetworkEdge &e: m_edges){
		if (e.m_nominalHeatingDemand <= 0){
			std::set<NetworkEdge *> edges1, edges2;
			const_cast<NetworkEdge&>(e).m_nominalHeatingDemand = 0.5 * ( e.m_node1->adjacentHeatingDemand(edges1)
										+ e.m_node2->adjacentHeatingDemand(edges2) );
		}
	}

	// fluid properties
	const double &rho = fluid.m_para[NetworkFluid::P_Density].value;
	const double &kinvis = fluid.m_kinematicViscosity.m_values.value(m_para[P_TemperatureSetpoint].get_value("C"));
	const double &cp = fluid.m_para[NetworkFluid::P_HeatCapacity].value;
	const double &lambda = fluid.m_para[NetworkFluid::P_Conductivity].value;

	// calculate temperature change indicator for each edge
	for (const NetworkEdge &e: m_edges){

		const NetworkPipe *pipe = pipeDB[e.m_idPipe];
		Q_ASSERT(pipe != nullptr);
		const_cast<NetworkEdge&>(e).m_nominalMassFlow = e.m_nominalHeatingDemand / (m_para[P_TemperatureDifference].get_value("K") * cp);

		double di = pipe->diameterInside();
		double area = PI/4 * di * di;
		double vel = e.m_nominalMassFlow / (rho * area);
		double re = IBK::ReynoldsNumber(vel, kinvis, di);
		double prandtl = IBK::PrandtlNumber(kinvis, cp, lambda, rho);
		double nusselt = IBK::NusseltNumber(re, prandtl, e.length(), pipe->diameterInside());
		double innerHeatTransferCoefficient = nusselt * lambda / pipe->diameterInside();
		double UAValue = e.length() /
				(
					  1.0 / ( innerHeatTransferCoefficient * pipe->diameterInside() * PI)
					+ 1.0 / pipe->UValue()
				);

		// is dimensionless or [K/K] for interpretation
		const_cast<NetworkEdge&>(e).m_tempChangeIndicator = UAValue / (e.m_nominalMassFlow * cp);
	}
}


void Network::findSourceNodes(std::vector<NetworkNode> &sources) const{
	for (const NetworkNode &n: m_nodes){
		if (n.m_type==NetworkNode::NT_Source)
			sources.push_back(n);
	}
}


void Network::dijkstraShortestPathToSource(const NetworkNode &startNode, const NetworkNode &endNode,
										   std::vector<NetworkEdge*> &pathEndToStart) const{

	// init: all nodes have infinte distance to start node and no predecessor
	for (const NetworkNode &n: m_nodes){
		n.m_distanceToStart = std::numeric_limits<double>::max();
		n.m_predecessor = nullptr;
	}
	startNode.m_distanceToStart = 0;
	std::set<unsigned> visitedNodes;

	// go through all not-visited nodes
	while (visitedNodes.size() <= m_nodes.size()){
		// find node with currently smallest distance to start, which has not yet been visited:
		double minDistance = std::numeric_limits<double>::max();
		const NetworkNode *nMin = nullptr;
		for (const NetworkNode &n: m_nodes){
			if (visitedNodes.find(n.m_id) == visitedNodes.end() && n.m_distanceToStart < minDistance){
				minDistance = n.m_distanceToStart;
				nMin = &n;
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

NetworkEdge * Network::edgeById(unsigned id) {
	for (NetworkEdge &e: m_edges){
		if (e.m_id == id)
			return &e;
	}
	IBK_ASSERT(false);
	return nullptr;
}


unsigned int VICUS::Network::indexOfEdge(unsigned nodeId1, unsigned nodeId2){
	for (unsigned int i=0; i<m_edges.size(); ++i){
		if (m_edges[i].nodeId1() == nodeId1 && m_edges[i].nodeId2() == nodeId2)
			return i;
	}
	IBK_ASSERT(false);
	return 9999;
}

unsigned int Network::indexOfEdge(unsigned edgeId) {
	for (unsigned int i=0; i<m_edges.size(); ++i){
		if (m_edges[i].m_id == edgeId)
			return i;
	}
	IBK_ASSERT(false);
	return 9999;
}

size_t Network::numberOfBuildings() const{
	size_t count = 0;
	for (const NetworkNode &n: m_nodes){
		if (n.m_type == NetworkNode::NT_SubStation)
			++count;
	}
	return count;
}

void Network::writeNetworkNodesCSV(const IBK::Path &file) const{
	std::ofstream f;
	f.open(file.str(), std::ofstream::out | std::ofstream::trunc);
	for (const NetworkNode &n: m_nodes){
		f.precision(0);
		f << std::fixed << n.m_id << "\t";
		f.precision(10);
		f << std::fixed << n.m_position.m_x << "\t" << n.m_position.m_y << "\t" << std::endl;
	}
	f.close();
}

void Network::writeNetworkEdgesCSV(const IBK::Path &file) const{
	std::ofstream f;
	f.open(file.str(), std::ofstream::out | std::ofstream::trunc);
	f.precision(0);
	for (const NetworkEdge &e: m_edges){
		f << std::fixed << e.m_idNodeInlet << "\t" <<e.m_idNodeOutlet << "\t" << e.m_idSoil;
		f.precision(10);
		f << "\t" << e.m_cumulativeTempChangeIndicator << "\t" << e.length() << std::endl;
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
		if (n.m_type==NetworkNode::NT_SubStation)
			f << std::fixed << n.m_position.m_x << "\t" << n.m_position.m_y << "\t" << n.m_maxHeatingDemand.value << std::endl;
	}
	f.close();
}



} // namespace VICUS
