#include "SVUndoNetworkAddPipeline.h"
#include "SVProjectHandler.h"

#include <VICUS_Project.h>

SVUndoNetworkAddPipeline::SVUndoNetworkAddPipeline(const std::vector<IBKMK::Vector3D> & polyLine, unsigned int pipeId, unsigned int networkId, bool findIntersections):
	m_polyLine(polyLine),
	m_pipeId(pipeId),
	m_networkId(networkId),
	m_findIntersections(findIntersections)
{
}

void SVUndoNetworkAddPipeline::undo() {
	VICUS::Network *net = dynamic_cast<VICUS::Network*>( theProject().objectById(m_networkId) );
	Q_ASSERT(net != nullptr);

	std::swap(*net, m_previousNetwork);

	// pointer update
	theProject().updatePointers();

	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkGeometryChanged);
}


void SVUndoNetworkAddPipeline::redo() {

	VICUS::Network *net = dynamic_cast<VICUS::Network*>( theProject().objectById(m_networkId) );
	Q_ASSERT(net != nullptr);

	m_previousNetwork = *net;

	// add all nodes as Mixer
	std::vector<unsigned int> addedNodeIds;
	std::vector<unsigned int> addedEdgeIds;
	unsigned int nextId = theProject().nextUnusedID();
	for (const IBKMK::Vector3D &vec: m_polyLine) {
		unsigned int nodeId = net->addNode(++nextId, vec, VICUS::NetworkNode::NT_Mixer);
		addedNodeIds.push_back(nodeId);
	}

	// add edges
	for (unsigned int i=1; i<addedNodeIds.size(); ++i){
		addedEdgeIds.push_back(++nextId);
		net->addEdge(addedEdgeIds.back(), addedNodeIds[i-1], addedNodeIds[i], true, m_pipeId);
	}

	net->updateNodeEdgeConnectionPointers();

	// add intersections
	if (m_findIntersections) {
		net->generateIntersections(++nextId, addedEdgeIds);
	}

	// pointer update
	theProject().updatePointers();

	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkGeometryChanged);
}
