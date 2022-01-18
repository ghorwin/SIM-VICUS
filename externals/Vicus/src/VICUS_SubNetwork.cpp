#include "VICUS_SubNetwork.h"

#include "VICUS_Project.h"
#include "VICUS_utilities.h"

#include <NANDRAD_HydraulicNetworkElement.h>

namespace VICUS {

bool SubNetwork::isValid(const Database<NetworkComponent> &compDB,
						 const Database<NetworkController> &ctrlDB,
						 const Database<Schedule> &scheduleDB) const
{
	if (m_elements.empty())
		return false;

	for (const NetworkElement &e: m_elements) {
		// check if we have valid ids
		if (e.m_id == INVALID_ID ||
			e.m_componentId == INVALID_ID ||
			e.m_inletNodeId == INVALID_ID ||
			e.m_outletNodeId == INVALID_ID)
			return false;

		// check if the component exists in DB
		if (compDB[e.m_componentId] == nullptr)
			return false;
		if (!compDB[e.m_componentId]->isValid(scheduleDB))
			return false;

		// if controller id exists, it must also reference a valid controller in DB
		if (e.m_controlElementId != INVALID_ID){
			if (ctrlDB[e.m_controlElementId] == nullptr)
				return false;
			if (!ctrlDB[e.m_controlElementId]->isValid(scheduleDB))
				return false;
		}
	}

	return true;
}


AbstractDBElement::ComparisonResult SubNetwork::equal(const AbstractDBElement *other) const {
	const SubNetwork * otherSub = dynamic_cast<const SubNetwork*>(other);
	if (otherSub == nullptr)
		return Different;

	if (m_elements.size() != otherSub->m_elements.size())
		return Different;

	for (unsigned int i=0; i<m_elements.size(); ++i){
		if (m_elements[i] != otherSub->m_elements[i])
			return Different;
	}

	if (m_displayName != other->m_displayName)
		return Different;

	//check meta data
	if (m_color != otherSub->m_color)
		return OnlyMetaDataDiffers;

	return Equal;
}


const NetworkComponent * SubNetwork::heatExchangeComponent(const Database<NetworkComponent> &compDB) const {
	const NetworkElement *elem = VICUS::element(m_elements, m_idHeatExchangeElement);
	if (elem == nullptr)
		return nullptr;

	return compDB[elem->m_componentId];
}


} // namespace VICUS
