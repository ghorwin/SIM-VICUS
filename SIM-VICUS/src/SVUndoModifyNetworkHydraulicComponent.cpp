#include "SVUndoModifyNetworkHydraulicComponent.h"

SVUndoModifyNetworkHydraulicComponent::SVUndoModifyNetworkHydraulicComponent(const QString & label, const unsigned int networkId,
																			 const NANDRAD::HydraulicNetworkComponent &component):
	m_newComponent(component),
	m_networkId(networkId)
{
	// save current (old) component. If it does not exists yet, set oldComponentId to invalid
	VICUS::Network * nw = theProject().element(theProject().m_geometricNetworks, m_networkId);
	IBK_ASSERT(nw != nullptr);
	NANDRAD::HydraulicNetworkComponent *comp = VICUS::Project::element(nw->m_hydraulicComponents, m_newComponent.m_id);
	if (comp != nullptr)
		m_oldComponent = *comp;
	else
		m_oldComponent.m_id = NANDRAD::INVALID_ID;
}

void SVUndoModifyNetworkHydraulicComponent::undo()
{
	VICUS::Network * nw = theProject().element(theProject().m_geometricNetworks, m_networkId);
	IBK_ASSERT(nw != nullptr);
	NANDRAD::HydraulicNetworkComponent *comp = VICUS::Project::element(nw->m_hydraulicComponents, m_newComponent.m_id);
	if(comp != nullptr)
		return;

	// component was existing already, just was modified
	if(m_oldComponent.m_id != NANDRAD::INVALID_ID)
		*comp= m_oldComponent;
	// component didnt exist before
	else
		nw->m_hydraulicComponents.pop_back();

	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkHydraulicComponentModified);
}

void SVUndoModifyNetworkHydraulicComponent::redo()
{
	VICUS::Network * nw = theProject().element(theProject().m_geometricNetworks, m_networkId);
	IBK_ASSERT(nw != nullptr);
	NANDRAD::HydraulicNetworkComponent *comp = VICUS::Project::element(nw->m_hydraulicComponents, m_newComponent.m_id);
	// component already existing, just modify it
	if(comp != nullptr)
		*comp = m_newComponent;
	// add new component
	else
		nw->m_hydraulicComponents.push_back(m_newComponent);

	SVProjectHandler::instance().setModified( SVProjectHandler::NetworkHydraulicComponentModified);
}
