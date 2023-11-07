#include "VICUS_AcousticSoundProtectionTemplate.h"

namespace VICUS {

AbstractDBElement::ComparisonResult AcousticSoundProtectionTemplate::equal(const AbstractDBElement *other) const {
    const AcousticSoundProtectionTemplate * otherTemplate = dynamic_cast<const AcousticSoundProtectionTemplate*>(other);
	if (otherTemplate == nullptr)
		return Different;

	// check meta data
	if (m_displayName != otherTemplate->m_displayName)
		return OnlyMetaDataDiffers;

	return Equal;
}


}
