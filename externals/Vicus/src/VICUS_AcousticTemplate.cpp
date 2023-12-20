#include "VICUS_AcousticTemplate.h"

namespace VICUS {

AbstractDBElement::ComparisonResult AcousticTemplate::equal(const AbstractDBElement *other) const {
	const AcousticTemplate * otherAcousticTemplate = dynamic_cast<const AcousticTemplate*>(other);
	if (otherAcousticTemplate == nullptr)
		return Different;

	// check meta data
	if (m_displayName != otherAcousticTemplate->m_displayName ||
		m_dataSource != otherAcousticTemplate->m_dataSource ||
		m_note != otherAcousticTemplate->m_note)
		return OnlyMetaDataDiffers;

	return Equal;
}


}
