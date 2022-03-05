#include "VICUS_GridPlane.h"

#include <IBK_Exception.h>

#include <IBKMK_3DCalculations.h>

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
	FUNCID(GridPlane::closestSnapPoint);
	if (m_nGridLines == 0)
		throw IBK::Exception("GridPlane not properly initialized.", FUNC_ID);

	// Note: we rely on valid m_localX, m_localY and m_normal vectors, all orthogonal and with magnitude 1

	IBK_ASSERT(m_nGridLines % 2 == 1); // must be an odd number!

	// Indexes of the grid lines, -(m_nGridLines-1)/2 ... +(m_nGridLines-1)/2
	int i = 0;
	int j = 0;
	int maxGridLineNum = (m_nGridLines-1)/2;

	// we first compute the projected plain coordinates
	double localX, localY;

	// special handling for horizontal grids (the vast majority of grids will be this way)
	if (m_normal.m_x == 0.0 && m_normal.m_y == 0.0) {
		localX = intersectionPoint.m_x;
		localY = intersectionPoint.m_y;
	}
	else {
		if (!planeCoordinates(m_offset, m_localX, m_localY,
							  intersectionPoint, localX, localY))
		{
			snapPoint = intersectionPoint;
			return;
		}
	}

	// now we check if we are outside our plane limits and clip the coordinates respectively
	double minorGridSpacing = m_spacing/10;
	if (localX < -m_gridExtends) {
		i = -maxGridLineNum;
	}
	else if (localX > m_gridExtends) {
		i = maxGridLineNum;
	}
	else {
		i = (int)std::floor(intersectionPoint.m_x / minorGridSpacing + 0.5);  // round real (up/down)
	}

	if (localY < -m_gridExtends) {
		j = -maxGridLineNum;
	}
	else if (localY > m_gridExtends) {
		j = maxGridLineNum;
	}
	else {
		j = (int)std::floor(intersectionPoint.m_y / minorGridSpacing + 0.5);  // round real (up/down)
	}

	// recompute snap coordinates
	snapPoint = m_offset + i*m_localX + j*m_localY;
}


} // namespace VICUS
