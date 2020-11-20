#include "SVUndoAddNetwork.h"
#include "SVProjectHandler.h"

SVUndoAddNetwork::SVUndoAddNetwork(	const QString & label,
								const VICUS::Network & addedNetwork) :
	m_addedNetwork(addedNetwork)
{
	setText( label );
}


void SVUndoAddNetwork::undo() {

	// remove last network
	Q_ASSERT(!theProject().m_networks.empty());

	theProject().m_networks.pop_back();

	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkModified);
}


void SVUndoAddNetwork::redo() {
	// append network
	theProject().m_networks.push_back(m_addedNetwork);
	theProject().m_networks.back().updateNodeEdgeConnectionPointers(); // ensure pointers are correctly set
	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkModified);
}
