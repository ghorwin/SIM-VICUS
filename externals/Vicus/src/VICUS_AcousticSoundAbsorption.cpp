#include "VICUS_AcousticSoundAbsorption.h"

#include <IBK_math.h>

namespace VICUS {

bool AcousticSoundAbsorption::isValid() const {

	for (unsigned int i=0; i<NUM_SF; ++i){
		if(m_soundAbsorption[(SoundAbsorptionFrequency)i] < 0)
			return false;
	}

	return true;
}

AbstractDBElement::ComparisonResult AcousticSoundAbsorption::equal(const AbstractDBElement *other) const
{
	const AcousticSoundAbsorption * otherSA = dynamic_cast<const AcousticSoundAbsorption*>(other);
	if(otherSA == nullptr)
		return Different;

	for(unsigned int i = 0; i < SoundAbsorptionFrequency::NUM_SF; ++i) {
		if(IBK::near_equal(m_soundAbsorption[(SoundAbsorptionFrequency)i],
						   otherSA->m_soundAbsorption[(SoundAbsorptionFrequency)i]))
			return Different;
	}

	if(m_displayName != otherSA->m_displayName)
		return AbstractDBElement::OnlyMetaDataDiffers;

	return Equal;
}

} // namespace VICUS
