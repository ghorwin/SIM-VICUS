#include "VICUS_Outputs.h"

namespace VICUS {

bool Outputs::operator!=(const Outputs & other) const {
	for (int i=0; i<NUM_OF; ++i)
		if (m_flags[i] != other.m_flags[i])
			return true;
	if (m_timeUnit != other.m_timeUnit)
		return true;
	if (m_definitions != other.m_definitions)
		return true;
	if (m_grids != other.m_grids)
		return true;

	return false;
}

} // namespace VICUS
