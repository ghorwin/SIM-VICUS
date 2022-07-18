#include "SVUndoNetworkAddPipeline.h"
#include "SVProjectHandler.h"

#include <VICUS_Project.h>

SVUndoNetworkAddPipeline::SVUndoNetworkAddPipeline(const std::vector<IBKMK::Vector3D> & polyLine, unsigned int pipeId, unsigned int networkId):
	m_polyLine(polyLine),
	m_pipeId(pipeId),
	m_networkId(networkId)
{
}

void SVUndoNetworkAddPipeline::undo() {
	VICUS::Network *net = dynamic_cast<VICUS::Network*>( theProject().objectById(m_networkId) );
	Q_ASSERT(net != nullptr);

	for (unsigned int edgeId: m_addedEdgeIds){
		net->m_edges.erase(net->m_edges.begin() + net->indexOfEdge(edgeId));
	}
	for (unsigned int nodeId: m_addedNodeIds){
		net->m_nodes.erase(net->m_nodes.begin() + net->indexOfNode(nodeId));
	}
	m_addedNodeIds.clear();
	m_addedEdgeIds.clear();

	net->updateNodeEdgeConnectionPointers();

	// color and pointer update
	net->setDefaultColors();
	theProject().updatePointers();

	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkGeometryChanged);
}


void SVUndoNetworkAddPipeline::redo() {

	m_addedNodeIds.clear();
	m_addedEdgeIds.clear();

	VICUS::Network *net = dynamic_cast<VICUS::Network*>( theProject().objectById(m_networkId) );
	Q_ASSERT(net != nullptr);

	// add all nodes as Mixer
	unsigned int nextId = theProject().nextUnusedID();
	for (const IBKMK::Vector3D &vec: m_polyLine) {
	unsigned int nodeId = net->addNode(++nextId, vec, VICUS::NetworkNode::NT_Mixer);
	m_addedNodeIds.push_back(nodeId);
	}

	// add edges
	for (unsigned int i=1; i<m_addedNodeIds.size(); ++i){
		net->addEdge(++nextId, m_addedNodeIds[i-1], m_addedNodeIds[i], true, m_pipeId);
		m_addedEdgeIds.push_back(nextId);
	}

//	// add intersections
//	net->updateNodeEdgeConnectionPointers();
//	net->generateIntersections(++nextId, m_addedNodeIds, m_addedEdgeIds);

	// color and pointer update
	net->setDefaultColors();
	theProject().updatePointers();

	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkGeometryChanged);
}
