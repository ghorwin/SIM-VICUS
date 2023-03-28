#include "VICUS_AcousticTemplate.h"

namespace VICUS {
AcousticTemplate::AcousticTemplate() {}


AbstractDBElement::ComparisonResult AcousticTemplate::equal(const AbstractDBElement *other) const {
    const AcousticTemplate * otherAcousticTemplate = dynamic_cast<const AcousticTemplate*>(other);
    if (otherAcousticTemplate == nullptr)
		return Different;

	// check meta data
    if (m_displayName != otherAcousticTemplate->m_displayName ||
        m_dataSource != otherAcousticTemplate->m_dataSource ||
        m_notes != otherAcousticTemplate->m_notes)
		return OnlyMetaDataDiffers;

	return Equal;
}


}
