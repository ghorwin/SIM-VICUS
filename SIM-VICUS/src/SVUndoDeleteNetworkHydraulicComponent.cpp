#include "SVUndoDeleteNetworkHydraulicComponent.h"

SVUndoDeleteNetworkHydraulicComponent::SVUndoDeleteNetworkHydraulicComponent(const QString & label, const unsigned int networkId,
																			 const NANDRAD::HydraulicNetworkComponent & component):
	m_component(component),
	m_networkId(networkId)
{
}

void SVUndoDeleteNetworkHydraulicComponent::undo()
{
	VICUS::Network * nw = theProject().element(theProject().m_geometricNetworks, m_networkId);
	IBK_ASSERT(nw != nullptr);
	nw->m_hydraulicComponents.push_back(m_component);

	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkHydraulicComponentModified);
}

void SVUndoDeleteNetworkHydraulicComponent::redo()
{
	VICUS::Network * nw = theProject().element(theProject().m_geometricNetworks, m_networkId);
	IBK_ASSERT(nw != nullptr);

	for (auto it = nw->m_hydraulicComponents.begin(); it != nw->m_hydraulicComponents.end(); ++it){
		if (it->m_id==m_component.m_id){
			nw->m_hydraulicComponents.erase(it);
			break;
		}
	}

	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkHydraulicComponentModified);
}
