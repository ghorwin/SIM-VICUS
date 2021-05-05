#include "VICUS_SubSurfaceComponent.h"

namespace VICUS {

bool SubSurfaceComponent::isValid(const VICUS::Database<VICUS::Window> & windows,
								  const VICUS::Database<VICUS::BoundaryCondition> & bcs) const
{
	try {

		// referenced construction instance exists and is valid?
		const Window * win = windows[m_idWindow];
		if (win == nullptr)
			return false;

		if (!win->isValid())
			return false;

		const BoundaryCondition *bcA = bcs[m_idSideABoundaryCondition];
		const BoundaryCondition *bcB = bcs[m_idSideBBoundaryCondition];

		if (bcA == nullptr && bcB == nullptr)
			return false;

		if (bcA != nullptr && !bcA->isValid())
			return false;

		if (bcB != nullptr && !bcB->isValid())
			return false;


		// TODO : Dirk, add other checks for valid/complete parametrization
		return true;

	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		return false;
	}
}

AbstractDBElement::ComparisonResult SubSurfaceComponent::equal(const AbstractDBElement *other) const {
	const SubSurfaceComponent * otherComp = dynamic_cast<const SubSurfaceComponent*>(other);
	if (otherComp == nullptr)
		return Different;

	// first check critical data

	if (m_idWindow != otherComp->m_idWindow ||
			m_idSideABoundaryCondition != otherComp->m_idSideABoundaryCondition||
			m_idSideBBoundaryCondition != otherComp->m_idSideBBoundaryCondition ||
			m_type != otherComp->m_type)
		return Different;

	//check meta data

	if (m_displayName != otherComp->m_displayName || m_color != otherComp->m_color)
		return OnlyMetaDataDiffers;

	return Equal;
}


} // namespace VICUS
