#include "VICUS_SubNetwork.h"

#include <NANDRAD_HydraulicNetworkElement.h>

namespace VICUS {

SubNetwork::SubNetwork()
{

}

bool SubNetwork::isValid() const
{
	return !m_elements.empty();
}

AbstractDBElement::ComparisonResult SubNetwork::equal(const AbstractDBElement *other) const
{
	const SubNetwork * otherSub = dynamic_cast<const SubNetwork*>(other);
	if (otherSub == nullptr)
		return Different;

	if (m_elements.size() != otherSub->m_elements.size())
		return Different;

	for (unsigned int i=0; i<m_elements.size(); ++i){
		if (m_elements[i] != otherSub->m_elements[i])
			return Different;
	}

	//check meta data
	if (m_displayName != otherSub->m_displayName ||
		m_color != otherSub->m_color)
		return OnlyMetaDataDiffers;

	return Equal;
}


} // Namespace VICUS
