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


} // namespace VICUS
