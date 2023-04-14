#include "VICUS_AcousticComponent.h"
#include "IBK_math.h"

namespace VICUS {


AbstractDBElement::ComparisonResult AcousticComponent::equal(const AbstractDBElement *other) const{
	const AcousticComponent * otherAC = dynamic_cast<const AcousticComponent*>(other);
	if (otherAC == nullptr)
		return Different;

	if (!IBK::near_equal(m_airSoundResistenceValue,otherAC->m_airSoundResistenceValue) || !IBK::near_equal(m_impactSoundValue, otherAC->m_impactSoundValue))
		return Different;

	if (	m_displayName != otherAC->m_displayName)
		return OnlyMetaDataDiffers;

	return Equal;
}

} // namespace VICUS
