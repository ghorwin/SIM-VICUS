#include "VICUS_AcousticBuildingTemplate.h"
namespace VICUS {


AbstractDBElement::ComparisonResult AcousticBuildingTemplate::equal(const AbstractDBElement *other) const{
	const AcousticBuildingTemplate * otherABT = dynamic_cast<const AcousticBuildingTemplate*>(other);
	if (otherABT == nullptr)
		return Different;

	if (m_idsTemplate.size() != otherABT->m_idsTemplate.size())
		return Different;

	// check if the entries are identical
	for(unsigned int i = 0; i < m_idsTemplate.size(); ++i){
		if(m_idsTemplate[i] != otherABT->m_idsTemplate[i])
			return Different;
	}

	if (	m_displayName != otherABT->m_displayName)
		return OnlyMetaDataDiffers;

	return Equal;
}

} // namespace VICUS
