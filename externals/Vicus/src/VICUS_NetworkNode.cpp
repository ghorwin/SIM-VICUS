#include "VICUS_Node.h"
#include "VICUS_NetworkEdge.h"

#include <IBK_assert.h>

namespace VICUS {

void Node::collectConnectedEdges(std::set<const Node *> & connectedNodes,
										  std::set<const Edge *> & connectedEdge) const {
	// store ourselves as connected
	connectedNodes.insert(this);
	// now ask connected elements to collect their nodes
	for (const Edge * e : m_edges) {
		// only process edges that are not yet collected
		if (connectedEdge.find(e) == connectedEdge.end())
			e->collectConnectedNodes(connectedNodes, connectedEdge);
	}
}


void Node::updateIsDeadEnd(){
	unsigned c = 0;
	for (Edge *e: m_edges){
		if (!e->neighbourNode(this)->m_isDeadEnd)
			++c;
	}
	m_isDeadEnd = c < 2 && m_type != NT_Building && m_type != NT_Source;
}


Edge *Node::neighborEdge(const Edge *e) const
{
	IBK_ASSERT(m_edges.size()==2);
	if (m_edges[0] == e)
		return m_edges[1];
	else
		return m_edges[0];
}


const Node * Node::findNextNonRedundantNode(std::set<unsigned> & redundantNodes, double & distance, const Edge *edgeToVisit) const
{
	distance += edgeToVisit->m_length;
	Node * nextNode = edgeToVisit->neighbourNode(this);
	if (!nextNode->isRedundant())
		return nextNode;
	redundantNodes.insert(nextNode->m_id);
	return nextNode->findNextNonRedundantNode(redundantNodes, distance, nextNode->neighborEdge(edgeToVisit));
}


void Node::findRedundantNodes(std::set<unsigned> & redundantNodes, std::set<const Edge *> & visitedEdges) const
{
	// redundant nodes have exactly 2 connected edges
	if (this->isRedundant()){
		redundantNodes.insert(this->m_id);
	}
	for (const Edge * e: m_edges){
		// remember visited edges and dont visit them again
		if (visitedEdges.find(e) == visitedEdges.end()){
			visitedEdges.insert(e);
			if (e->m_nodeId1 == this->m_id)
				e->m_node2->findRedundantNodes(redundantNodes, visitedEdges);
			else
				e->m_node1->findRedundantNodes(redundantNodes, visitedEdges);
		}
	}
}


bool Node::findPathToSource(std::set<Edge*> &path, std::set<Edge*> &visitedEdges, std::set<unsigned> &visitedNodes){

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


void Node::updateNeighbourDistances() {
	// calculate alternative distance to neighbour.
	// If it is shorter than current one: set it as its current distance
	for (Edge *e :m_edges){
		Node * neighbour = e->neighbourNode(this);
		double alternativeDistance = m_distanceToStart + e->m_length;
		if (alternativeDistance < neighbour->m_distanceToStart){
			neighbour->m_distanceToStart = alternativeDistance;
			neighbour->m_predecessor = this;
		}
	}
}


void Node::pathToNull(std::vector<Edge*> &path){
	if (m_predecessor == nullptr)
		return;
	for (Edge * e: m_edges){
		if (e->neighbourNode(this) == m_predecessor){
			path.push_back(e);
			e->neighbourNode(this)->pathToNull(path);
		}
	}
}

double Node::adjacentHeatingDemand(std::set<Edge *> visitedEdges){
	for (Edge *e: m_edges){
		if (visitedEdges.find(e)==visitedEdges.end()){
			visitedEdges.insert(e);
			if (e->m_heatingDemand>0)
				return e->m_heatingDemand;
			e->neighbourNode(this)->adjacentHeatingDemand(visitedEdges);
		}
	}
}

}
