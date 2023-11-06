#include "VICUS_AcousticTemplate.h"

namespace VICUS {

AbstractDBElement::ComparisonResult AcousticTemplate::equal(const AbstractDBElement *other) const {
	const AcousticTemplate * otherEPD = dynamic_cast<const AcousticTemplate*>(other);
	if (otherEPD == nullptr)
		return Different;

	// check meta data
	if (m_displayName != otherEPD->m_displayName ||
		m_dataSource != otherEPD->m_dataSource ||
		m_notes != otherEPD->m_notes)
		return OnlyMetaDataDiffers;

	return Equal;
}


}
