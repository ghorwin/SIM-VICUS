#include "SVUndoModifyExistingNetwork.h"

#include "SVSettings.h"

SVUndoModifyExistingNetwork::SVUndoModifyExistingNetwork(const QString &label, unsigned int networkIndex, const VICUS::Network & modNetwork) :
	m_networkIndex(networkIndex),
	m_network(modNetwork)
{
	setText( label );
}


void SVUndoModifyExistingNetwork::undo() {
	IBK_ASSERT(m_networkIndex < project().m_geometricNetworks.size());
	std::swap(theProject().m_geometricNetworks[m_networkIndex], m_network); // exchange network in project with network stored in this class
	theProject().m_geometricNetworks[m_networkIndex].updateNodeEdgeConnectionPointers();
	const SVDatabase & db = SVSettings::instance().m_db;
	theProject().m_geometricNetworks[m_networkIndex].updateVisualizationData(db.m_pipes);

	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkModified);
}


void SVUndoModifyExistingNetwork::redo() {
	undo(); // same as undo
}
