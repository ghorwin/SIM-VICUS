#include "SVUndoDeleteNetwork.h"

SVUndoDeleteNetwork::SVUndoDeleteNetwork(const QString & label, unsigned int networkIndex)
	: m_networkIndex(networkIndex)
{
	setText( label );

	Q_ASSERT(project().m_geometricNetworks.size() > networkIndex);

	m_deletedNetwork = project().m_geometricNetworks[networkIndex];
}


void SVUndoDeleteNetwork::undo() {

	theProject().m_geometricNetworks.insert(theProject().m_geometricNetworks.begin() + m_networkIndex, m_deletedNetwork);
	theProject().updatePointers();

	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkModified);
}


void SVUndoDeleteNetwork::redo() {
	Q_ASSERT(!theProject().m_geometricNetworks.empty());

	theProject().m_geometricNetworks.erase(theProject().m_geometricNetworks.begin() + m_networkIndex);
	theProject().updatePointers();

	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkModified);
}

