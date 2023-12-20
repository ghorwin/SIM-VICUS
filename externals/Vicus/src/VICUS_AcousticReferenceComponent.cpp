#include "VICUS_AcousticReferenceComponent.h"
namespace VICUS {


AbstractDBElement::ComparisonResult AcousticReferenceComponent::equal(const AbstractDBElement *other) const{
	const AcousticReferenceComponent * otherARC = dynamic_cast<const AcousticReferenceComponent*>(other);
	if (otherARC == nullptr)
		return Different;

	if (m_buildingType != otherARC->m_buildingType || m_idAcousticTemplateA != otherARC->m_idAcousticTemplateA
			|| m_idAcousticTemplateB != otherARC->m_idAcousticTemplateB || m_requirementType != otherARC->m_requirementType)
		return Different;

	if(!IBK::near_equal(m_impactSoundOneStructureUnit, otherARC->m_impactSoundOneStructureUnit)|| !IBK::near_equal(m_impactSoundDifferentStructureUnit, otherARC->m_impactSoundDifferentStructureUnit) ||
			!IBK::near_equal(m_airborneSoundOneStructureUnit, otherARC->m_airborneSoundOneStructureUnit) ||!IBK::near_equal( m_airborneSoundDifferentStructureUnit, otherARC->m_airborneSoundDifferentStructureUnit))
		return Different;

	if (m_displayName != otherARC->m_displayName)
		return OnlyMetaDataDiffers;

	return Equal;
}

}
