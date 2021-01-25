#include "NANDRAD_HydraulicNetworkElement.h"

#include <NANDRAD_KeywordList.h>

namespace NANDRAD {

HydraulicNetworkElement::HydraulicNetworkElement(unsigned int id, unsigned int inletNodeId, unsigned int outletNodeId,
						unsigned int componentId, unsigned int pipeID, double length) :
	m_id(id),
	m_inletNodeId(inletNodeId),
	m_outletNodeId(outletNodeId),
	m_componentId(componentId),
	m_pipePropertiesId(pipeID)
{
	NANDRAD::KeywordList::setParameter(m_para, "HydraulicNetworkElement::para_t", P_Length, length);
}


void HydraulicNetworkElement::checkParameters(int networkModelType) const {

}


} // namespace NANDRAD
