#include "SVUndoAddNetwork.h"
#include "SVProjectHandler.h"
#include "SVSettings.h"

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
	Q_ASSERT(!theProject().m_geometricNetworks.empty());

	theProject().m_geometricNetworks.pop_back();
	if (theProject().m_geometricNetworks.empty())
		return;
	theProject().m_geometricNetworks.back().updateNodeEdgeConnectionPointers(); // ensure pointers are correctly set
	const SVDatabase & db = SVSettings::instance().m_db;
	theProject().m_geometricNetworks.back().updateVisualizationData(db.m_pipes);

	std::swap(theProject().m_viewSettings.m_gridWidth, m_gridWidth);
	std::swap(theProject().m_viewSettings.m_gridSpacing, m_gridSpacing);
	std::swap(theProject().m_viewSettings.m_farDistance, m_farDistance);

	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkModified);
	SVProjectHandler::instance().setModified( SVProjectHandler::GridModified);
}


void SVUndoAddNetwork::redo() {
	// append network

	theProject().m_geometricNetworks.push_back(m_addedNetwork);
	theProject().updatePointers();
	std::swap(theProject().m_viewSettings.m_gridWidth, m_gridWidth);
	std::swap(theProject().m_viewSettings.m_gridSpacing, m_gridSpacing);
	std::swap(theProject().m_viewSettings.m_farDistance, m_farDistance);

	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkModified);
	SVProjectHandler::instance().setModified( SVProjectHandler::GridModified);
}

