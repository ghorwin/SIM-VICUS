#include "SVUndoNetworkAddNodes.h"
#include "SVProjectHandler.h"

#include <VICUS_Project.h>


SVUndoNetworkAddNodes::SVUndoNetworkAddNodes(const std::vector<VICUS::NetworkNode> &nodes, unsigned int networkId):
	m_nodes(nodes),
	m_networkId(networkId)
{
}

void SVUndoNetworkAddNodes::undo() {
	VICUS::Network *net = dynamic_cast<VICUS::Network*>( theProject().objectById(m_networkId) );
	Q_ASSERT(net != nullptr);

	for (unsigned int nodeId: m_addedNodeIds){
		net->m_nodes.erase(net->m_nodes.begin() + net->indexOfNode(nodeId));
	}
	m_addedNodeIds.clear();

	net->updateNodeEdgeConnectionPointers();

	// pointer update
	theProject().updatePointers();

	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkGeometryChanged);
}


void SVUndoNetworkAddNodes::redo() {

	m_addedNodeIds.clear();

	VICUS::Network *net = dynamic_cast<VICUS::Network*>( theProject().objectById(m_networkId) );
	Q_ASSERT(net != nullptr);

	unsigned int nextId = theProject().nextUnusedID();
	for (const VICUS::NetworkNode &node: m_nodes) {
		unsigned int nodeId = net->addNode(++nextId, node.m_position, node.m_type, true);
		net->nodeById(nodeId)->m_displayName = node.m_displayName;
		net->nodeById(nodeId)->m_maxHeatingDemand = node.m_maxHeatingDemand;
		m_addedNodeIds.push_back(nodeId);
	}

	// pointer update
	theProject().updatePointers();

	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkGeometryChanged);
}
