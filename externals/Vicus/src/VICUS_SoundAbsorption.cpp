#include "VICUS_SoundAbsorption.h"

namespace VICUS {

AbstractDBElement::ComparisonResult SoundAbsorption::equal(const AbstractDBElement *other) const
{
	const SoundAbsorption * otherSA = dynamic_cast<const SoundAbsorption*>(other);
	if(otherSA == nullptr)
		return Different;

    for(int i = 0; i < SoundAbsorptionFrequency::NUM_SF; ++i)
    {
        if(m_soundAbsorption[i] != otherSA->m_soundAbsorption[i])
            return Different;
    }

	if(m_displayName != otherSA->m_displayName)
		return AbstractDBElement::OnlyMetaDataDiffers;
	return Equal;
}

} // namespace VICUS
