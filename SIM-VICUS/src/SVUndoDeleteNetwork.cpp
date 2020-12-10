#include "SVUndoDeleteNetwork.h"

SVUndoDeleteNetwork::SVUndoDeleteNetwork(const QString & label, const VICUS::Network & deletedNetwork):
m_deletedNetwork(deletedNetwork)
{
setText( label );
m_gridWidth = std::max(m_deletedNetwork.m_extends.width(), m_deletedNetwork.m_extends.height());
m_gridWidth = std::max(100., m_gridWidth);
m_gridSpacing = 100; // 100 m major grid
m_farDistance = 2*m_gridWidth;
}


void SVUndoDeleteNetwork::undo() {

	theProject().m_geometricNetworks.push_back(m_deletedNetwork);
	theProject().updatePointers();

	std::swap(theProject().m_viewSettings.m_gridWidth, m_gridWidth);
	std::swap(theProject().m_viewSettings.m_gridSpacing, m_gridSpacing);
	std::swap(theProject().m_viewSettings.m_farDistance, m_farDistance);

	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkModified);
	SVProjectHandler::instance().setModified( SVProjectHandler::GridModified);
}


void SVUndoDeleteNetwork::redo() {
	Q_ASSERT(!theProject().m_geometricNetworks.empty());

	for (auto it = theProject().m_geometricNetworks.begin(); it != theProject().m_geometricNetworks.end(); ++it){
		if (it->m_id == m_deletedNetwork.m_id){
			theProject().m_geometricNetworks.erase(it);
			break;
		}
	}
	theProject().updatePointers();

	std::swap(theProject().m_viewSettings.m_gridWidth, m_gridWidth);
	std::swap(theProject().m_viewSettings.m_gridSpacing, m_gridSpacing);
	std::swap(theProject().m_viewSettings.m_farDistance, m_farDistance);

	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkModified);
	SVProjectHandler::instance().setModified( SVProjectHandler::GridModified);
}

