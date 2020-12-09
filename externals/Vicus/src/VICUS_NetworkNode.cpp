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


void NetworkNode::updateIsDeadEnd(){
	unsigned numDeadEndNodes = 0;
	for (NetworkEdge *e: m_edges){
		if (!e->neighbourNode(this)->m_isDeadEnd)
			++numDeadEndNodes;
	}
	m_isDeadEnd = numDeadEndNodes < 2 && m_type != NT_Building && m_type != NT_Source;
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


void NetworkNode::updateNeighbourDistances() {
	// calculate alternative distance to neighbour.
	// If it is shorter than current one: set it as its current distance
	for (NetworkEdge *e :m_edges){
		NetworkNode * neighbour = e->neighbourNode(this);
		double alternativeDistance = m_distanceToStart + e->length();
		if (alternativeDistance < neighbour->m_distanceToStart){
			neighbour->m_distanceToStart = alternativeDistance;
			neighbour->m_predecessor = this;
		}
	}
}


void NetworkNode::pathToNull(std::vector<NetworkEdge*> &path){
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
			if (e->m_maxHeatingDemand>0)
				return e->m_maxHeatingDemand;
			e->neighbourNode(this)->adjacentHeatingDemand(visitedEdges);
		}
	}
}

}
