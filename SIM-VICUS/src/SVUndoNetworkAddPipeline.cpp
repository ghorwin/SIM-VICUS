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

	std::swap(*net, m_previousNetwork);

	// color and pointer update
	net->setDefaultColors();
	theProject().updatePointers();

	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkGeometryChanged);
}


void SVUndoNetworkAddPipeline::redo() {

	VICUS::Network *net = dynamic_cast<VICUS::Network*>( theProject().objectById(m_networkId) );
	Q_ASSERT(net != nullptr);

	m_previousNetwork = *net;

	// add all nodes as Mixer
	std::vector<unsigned int> addedNodeIds;
	unsigned int nextId = theProject().nextUnusedID();
	for (const IBKMK::Vector3D &vec: m_polyLine) {
		unsigned int nodeId = net->addNode(++nextId, vec, VICUS::NetworkNode::NT_Mixer);
		addedNodeIds.push_back(nodeId);
	}

	// add edges
	for (unsigned int i=1; i<addedNodeIds.size(); ++i){
		net->addEdge(++nextId, addedNodeIds[i-1], addedNodeIds[i], true, m_pipeId);
	}

//	// add intersections
//	net->updateNodeEdgeConnectionPointers();
//	net->generateIntersections(++nextId, m_addedNodeIds, m_addedEdgeIds);

	// color and pointer update
	net->setDefaultColors();
	theProject().updatePointers();

	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkGeometryChanged);
}
