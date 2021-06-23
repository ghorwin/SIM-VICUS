#ifndef SH_ShadedSurfaceObjectH
#define SH_ShadedSurfaceObjectH

#include "SH_Polygon.h"

#include <IBK_NotificationHandler.h>

namespace SH {

/*! Stores all data of a single surface, including its discretization. */
class ShadedSurfaceObject {
public:

	/*! Computes discretization for this surface and populates m_gridPoints vector. */
	void setPolygon(const SH::Polygon & surface, double gridWidth);

	/*! Computes and returns shading factor for the given sun normal vector. */
	double calcShadingFactor(const IBKMK::Vector3D &sunNormal, const std::vector<Polygon> & m_obstacles) const;

	std::vector<IBKMK::Vector3D>	m_gridPoints;
	std::vector<double >			m_gridAreas; // Later
};

} // namespace SH


#endif // SH_ShadedSurfaceObjectH
