#include "VICUS_GridPlane.h"

#include <IBK_Exception.h>

namespace VICUS {

bool GridPlane::intersectsLine(const IBKMK::Vector3D & p, const IBKMK::Vector3D & direction,
									  double & t,
									  IBKMK::Vector3D & intersectionPoint) const
{
	if (!m_isActive)
		return false;

	FUNCID(VICUS::GridPlane::intersectsLine);
	if (!IBK::nearly_equal<6>(m_normal.magnitudeSquared(), 1.0))
		throw IBK::Exception("Normal vector does not have unit length!", FUNC_ID);

	double d_dot_normal = direction.scalarProduct(m_normal);
	double angle = d_dot_normal/direction.magnitude();
	// line parallel to plane? no intersection
	if (angle < 1e-8 && angle > -1e-8)
		return false;

	// Condition 1: same direction of normal vectors?
	// -> does not apply to grid planes

	const IBKMK::Vector3D & planeOffset = m_offset;
	t = (m_offset - p).scalarProduct(m_normal) / d_dot_normal;

	// Condition 2: outside viewing range?
	if (t < 0 || t > 1)
		return false;

	// now determine location on plane
	IBKMK::Vector3D x0 = p + t*direction;

	// plane is endless - return intersection point and normalized distance t (no hole checking here!)
	intersectionPoint = x0;

	// determine snap point
	return true;
}


void GridPlane::closestSnapPoint(const IBKMK::Vector3D & intersectionPoint, IBKMK::Vector3D & snapPoint) const {

	// Indexes of the grid lines
	unsigned int i = 0;
	unsigned int j = 0;

	if (intersectionPoint.m_x > m_maxGrid)
		i = m_gridLineCount-1;
	else if (intersectionPoint.m_x > m_minGrid) {
		double offset = intersectionPoint.m_x - m_minGrid;
		i = (unsigned int)std::floor(offset/m_step);
		if (offset - i*m_step > m_step/2.0)
			++i;
	}
	if (intersectionPoint.m_y > m_maxGrid)
		j = m_gridLineCount-1;
	else if (intersectionPoint.m_y > m_minGrid) {
		double offset = intersectionPoint.m_y - m_minGrid;
		j = (unsigned int)std::floor(offset/m_step);
		if (offset - j*m_step > m_step/2.0)
			++j;
	}

	// recompute snap coordinates
	snapPoint = m_offset + i*m_localX + j*m_localY;
}

} // namespace VICUS
