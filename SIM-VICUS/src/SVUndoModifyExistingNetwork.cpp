#include "SVUndoModifyExistingNetwork.h"

SVUndoModifyExistingNetwork::SVUndoModifyExistingNetwork(const QString &label, const VICUS::Network &modNetwork):
	m_oldNetwork(*theProject().element(theProject().m_geometricNetworks, modNetwork.m_id)),
	m_newNetwork(modNetwork)
{
	setText( label );
	m_gridWidth = std::max(m_newNetwork.m_extends.width(), m_newNetwork.m_extends.height());
	m_gridWidth = std::max(100., m_gridWidth);
	m_gridSpacing = 100; // 100 m major grid
	m_farDistance = 2*m_gridWidth;
}

void SVUndoModifyExistingNetwork::undo()
{
	VICUS::Network * nw = theProject().element(theProject().m_geometricNetworks, m_newNetwork.m_id);
	IBK_ASSERT(nw != nullptr);
	*nw = m_newNetwork;
	nw->updateNodeEdgeConnectionPointers();

	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkModified);
}

void SVUndoModifyExistingNetwork::redo()
{
	VICUS::Network * nw = theProject().element(theProject().m_geometricNetworks, m_newNetwork.m_id);
	IBK_ASSERT(nw != nullptr);
	*nw = m_newNetwork;
	nw->updateNodeEdgeConnectionPointers();

	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkModified);
}
