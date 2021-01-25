#include "SVUndoModifyNetworkHydraulicComponent.h"

SVUndoModifyNetworkHydraulicComponent::SVUndoModifyNetworkHydraulicComponent(const QString & label,
		const unsigned int networkId, const unsigned int componentIndex, const NANDRAD::HydraulicNetworkComponent &comp) :
	m_component(comp),
	m_networkId(networkId),
	m_componentIndex(componentIndex)
{
	setText( label );
}


void SVUndoModifyNetworkHydraulicComponent::undo() {
	VICUS::Network * nw = theProject().element(theProject().m_geometricNetworks, m_networkId);
	IBK_ASSERT(nw != nullptr);
	IBK_ASSERT(nw->m_hydraulicComponents.size() > m_componentIndex);

	// swap stored and new component
	std::swap(m_component, nw->m_hydraulicComponents[m_componentIndex]);

	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkHydraulicComponentModified);
}


void SVUndoModifyNetworkHydraulicComponent::redo() {
	undo(); // same code as undo
}
