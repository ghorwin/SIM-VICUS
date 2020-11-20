#include "SVUndoAddNetwork.h"
#include "SVProjectHandler.h"

SVUndoAddNetwork::SVUndoAddNetwork(	const QString & label,
								const VICUS::Network & addedNetwork) :
	m_addedNetwork(addedNetwork)
{
	setText( label );
	m_gridWidth = std::max(addedNetwork.m_extends.width(), addedNetwork.m_extends.height());
	m_gridWidth = std::max(100., m_gridWidth);
	m_gridSpacing = 100; // 100 m major grid
	m_farDistance = 2*m_gridWidth;
}


void SVUndoAddNetwork::undo() {

	// remove last network
	Q_ASSERT(!theProject().m_networks.empty());

	theProject().m_networks.pop_back();

	std::swap(theProject().m_viewSettings.m_gridWidth, m_gridWidth);
	std::swap(theProject().m_viewSettings.m_gridSpacing, m_gridSpacing);
	std::swap(theProject().m_viewSettings.m_farDistance, m_farDistance);

	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkModified);
	SVProjectHandler::instance().setModified( SVProjectHandler::GridModified);
}


void SVUndoAddNetwork::redo() {
	// append network
	theProject().m_networks.push_back(m_addedNetwork);
	theProject().m_networks.back().updateNodeEdgeConnectionPointers(); // ensure pointers are correctly set
	std::swap(theProject().m_viewSettings.m_gridWidth, m_gridWidth);
	std::swap(theProject().m_viewSettings.m_gridSpacing, m_gridSpacing);
	std::swap(theProject().m_viewSettings.m_farDistance, m_farDistance);

	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkModified);
	SVProjectHandler::instance().setModified( SVProjectHandler::GridModified);
}
