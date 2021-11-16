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

#include "VICUS_NetworkNode.h"
#include "VICUS_NetworkEdge.h"

#include <IBK_assert.h>

namespace VICUS {

void NetworkNode::collectConnectedEdges(std::set<const NetworkNode *> & connectedNodes,
										std::set<const NetworkEdge *> & connectedEdge) const {
	// store ourselves as connected
	connectedNodes.insert(this);
	// now ask connected elements to collect their nodes
	for (const NetworkEdge * e : m_edges) {
		// only process edges that are not yet collected
		if (connectedEdge.find(e) == connectedEdge.end())
			e->collectConnectedNodes(connectedNodes, connectedEdge);
	}
}


void NetworkNode::setInletOutletNode(std::set<const NetworkNode *> &visitedNodes, std::vector<const NetworkEdge *> &orderedEdges) const {
	// store ourselves as connected
	visitedNodes.insert(this);
	// now ask connected elements to collect their nodes
	for (NetworkEdge * e : m_edges) {
		// only process edges that are not yet collected
		if (std::find(orderedEdges.begin(), orderedEdges.end(), e) == orderedEdges.end()){
			e->m_idNodeInlet = m_id;
			e->m_idNodeOutlet = e->neighbourNode(m_id);
			e->setInletOutletNode(visitedNodes, orderedEdges);
		}
	}
}


void NetworkNode::updateIsDeadEnd(){
	// buildings or sources are never dead ends
	if (m_type == NT_Building || m_type == NT_Source){
		m_isDeadEnd = false;
		return;
	}
	// check all neighbouring nodes. If only 1 or less are not dead ends: this is a dead end
	unsigned numNonDeadEndNeighbours = 0;
	for (NetworkEdge *e: m_edges){
		if (!e->neighbourNode(this)->m_isDeadEnd)
			++numNonDeadEndNeighbours;
	}
	m_isDeadEnd = numNonDeadEndNeighbours <= 1;
}


NetworkEdge * NetworkNode::neighborEdge(const NetworkEdge *e) const
{
	IBK_ASSERT(m_edges.size()==2);
	if (m_edges[0] == e)
		return m_edges[1];
	else
		return m_edges[0];
}


const NetworkNode * NetworkNode::findNextNonRedundantNode(std::set<unsigned> & redundantNodes, double & distance, const NetworkEdge *edgeToVisit) const
{
	distance += edgeToVisit->length();
	NetworkNode * nextNode = edgeToVisit->neighbourNode(this);
	if (!nextNode->isRedundant())
		return nextNode;
	redundantNodes.insert(nextNode->m_id);
	return nextNode->findNextNonRedundantNode(redundantNodes, distance, nextNode->neighborEdge(edgeToVisit));
}


void NetworkNode::findRedundantNodes(std::set<unsigned> & redundantNodes, std::set<const NetworkEdge *> & visitedEdges) const
{
	// redundant nodes have exactly 2 connected edges
	if (this->isRedundant()){
		redundantNodes.insert(this->m_id);
	}
	for (const NetworkEdge * e: m_edges){
		// remember visited edges and dont visit them again
		if (visitedEdges.find(e) == visitedEdges.end()){
			visitedEdges.insert(e);
			if (e->nodeId1() == this->m_id)
				e->m_node2->findRedundantNodes(redundantNodes, visitedEdges);
			else
				e->m_node1->findRedundantNodes(redundantNodes, visitedEdges);
		}
	}
}


bool NetworkNode::findPathToSource(std::set<NetworkEdge*> &path, std::set<NetworkEdge*> &visitedEdges, std::set<unsigned> &visitedNodes){

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

	for (NetworkEdge *e: m_edges){
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


void NetworkNode::updateNeighbourDistances() const{
	// calculate alternative distance to neighbour.
	// If it is shorter than current one: set it as its current distance
	for (const NetworkEdge *e :m_edges){
		NetworkNode * neighbour = e->neighbourNode(this);
		double alternativeDistance = m_distanceToStart + e->length();
		if (alternativeDistance < neighbour->m_distanceToStart){
			neighbour->m_distanceToStart = alternativeDistance;
			neighbour->m_predecessor = this;
		}
	}
}


void NetworkNode::pathToNull(std::vector<NetworkEdge*> &path) const{
	if (m_predecessor == nullptr)
		return;
	for (NetworkEdge * e: m_edges){
		if (e->neighbourNode(this) == m_predecessor){
			path.push_back(e);
			e->neighbourNode(this)->pathToNull(path);
		}
	}
}

double NetworkNode::adjacentHeatingDemand(std::set<NetworkEdge *> visitedEdges){
	for (NetworkEdge *e: m_edges){
		if (visitedEdges.find(e)==visitedEdges.end()){
			visitedEdges.insert(e);
			if (e->m_nominalHeatingDemand>0)
				return e->m_nominalHeatingDemand;
			e->neighbourNode(this)->adjacentHeatingDemand(visitedEdges);
		}
	}
	return 0;
}

}
