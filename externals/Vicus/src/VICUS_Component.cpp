#include "VICUS_Component.h"

namespace VICUS {

bool Component::isValid(const VICUS::Database<Material> & materials, const VICUS::Database<Construction> & constructions,
						const VICUS::Database<BoundaryCondition> & bcs) const
{
	try {

		// referenced construction instance exists and is valid?
		const Construction * con = constructions[m_idConstruction];
		if (con == nullptr)
			return false;

		if (!con->isValid(materials))
			return false;

		const BoundaryCondition *bcA = bcs[m_idSideABoundaryCondition];
		const BoundaryCondition *bcB = bcs[m_idSideBBoundaryCondition];

		if(bcA == nullptr && bcB == nullptr)
			return false;

		if(bcA != nullptr && !bcA->isValid())
			return false;

		if(bcB != nullptr && !bcB->isValid())
			return false;


		// TODO : Dirk, add other checks for valid/complete parametrization
		return true;

	} catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		return false;
	}
}

AbstractDBElement::ComparisonResult Component::equal(const AbstractDBElement *other) const {
	const Component * otherComp = dynamic_cast<const Component*>(other);
	if (otherComp == nullptr)
		return Different;

	//first check critical data

	if(m_idConstruction != otherComp->m_idConstruction ||
			m_idSideABoundaryCondition != otherComp->m_idSideABoundaryCondition||
			m_idSideBBoundaryCondition != otherComp->m_idSideBBoundaryCondition ||
			m_idSurfaceProperty != otherComp->m_idSurfaceProperty ||
			m_idWindow != otherComp->m_idWindow ||
			m_type != otherComp->m_type)
		return Different;

	//check meta data

	if(m_displayName != otherComp->m_displayName ||
			m_color != otherComp->m_color ||
			m_dataSource != otherComp->m_dataSource ||
			m_manufacturer != otherComp->m_manufacturer ||
			m_notes != otherComp->m_notes)
		return OnlyMetaDataDiffers;

	return Equal;
}


} // namespace VICUS
