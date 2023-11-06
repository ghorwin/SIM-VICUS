#include "VICUS_SoundProtectionTemplate.h"

namespace VICUS {

AbstractDBElement::ComparisonResult SoundProtectionTemplate::equal(const AbstractDBElement *other) const {
	const SoundProtectionTemplate * otherTemplate = dynamic_cast<const SoundProtectionTemplate*>(other);
	if (otherTemplate == nullptr)
		return Different;

	// check meta data
	if (m_displayName != otherTemplate->m_displayName)
		return OnlyMetaDataDiffers;

	return Equal;
}


}
