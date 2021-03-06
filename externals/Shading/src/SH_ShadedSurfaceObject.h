#ifndef SH_ShadedSurfaceObjectH
#define SH_ShadedSurfaceObjectH

#include <IBKMK_Polygon3D.h>

#include <IBK_NotificationHandler.h>

namespace SH {

/*! Stores all data of a single surface, including its discretization. */
class ShadedSurfaceObject {
public:

	/*! Computes discretization for this surface and populates m_gridPoints vector. */
	void setPolygon(const IBKMK::Polygon3D & surface, double gridWidth);

	/*! Computes and returns shading factor for the given sun normal vector. */
	double calcShadingFactor(const IBKMK::Vector3D &sunNormal, const std::vector<IBKMK::Polygon3D> & m_obstacles) const;

	std::vector<IBKMK::Vector3D>	m_gridPoints;
	std::vector<double >			m_gridAreas; // Later

	double							m_minX;
	double							m_maxX;
	double							m_minY;
	double							m_maxY;
};

} // namespace SH


#endif // SH_ShadedSurfaceObjectH
