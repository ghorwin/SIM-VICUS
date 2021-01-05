#include "VICUS_Component.h"

namespace VICUS {

bool Component::isValid(const VICUS::Database<Material> & materials, const VICUS::Database<Construction> & constructions,
						const VICUS::Database<BoundaryCondition> & bcs) const
{
	// referenced construction instance exists and is valid?
	const Construction * con = constructions[m_idOpaqueConstruction];
	if (con == nullptr)
		return false;

	if (!con->isValid(materials))
		return false;

	// TODO : Dirk, add other checks for valid/complete parametrization
	return true;
}


} // namespace VICUS
