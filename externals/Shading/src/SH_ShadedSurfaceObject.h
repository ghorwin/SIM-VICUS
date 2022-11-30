#ifndef SH_ShadedSurfaceObjectH
#define SH_ShadedSurfaceObjectH

#include <IBKMK_Polygon3D.h>

#include <IBK_NotificationHandler.h>

#include <clipper.hpp>

#include "SH_StructuralShading.h"


namespace SH {


/*! Stores all data of a single surface, including its discretization. */
class ShadedSurfaceObject {
public:
	/*! Computes discretization for this surface and populates m_gridPoints vector. */
	void setPolygon(unsigned int id, std::string name, const IBKMK::Polygon3D & surface, const std::vector<IBKMK::Polygon2D> &holes,
					unsigned int idParent = INVALID_ID, double gridWidth = 0.1, bool useClipping = false);

	/*! Computes and returns shading factor for the given sun normal vector. */
	double calcShadingFactorWithRayTracing(const IBKMK::Vector3D &sunNormal, const std::vector<StructuralShading::ShadingObject> & obstacles) const;

	/*! Computes and returns shading factor for the given sun normal vector. */
	double calcShadingFactorWithClipping(unsigned int idxSun, const IBKMK::Vector3D & sunNormal,
										 const std::vector<StructuralShading::ShadingObject> & obstacles) const;

	/*! Updates the projected polygon 2D. */
	void setProjectedPolygonAndHoles(const std::vector<IBKMK::Vector2D> & poly, const std::vector<std::vector<IBKMK::Vector2D> > & holes);

	void setOutputFile(std::ofstream * newOutputFile);

private:

	void addAreaOfPolyNode(const ClipperLib::PolyNode *polyNode, double &area) const;
	void writePathToOutputFile(const std::string preText, const ClipperLib::Path &path) const;

	std::vector<IBKMK::Vector3D>				m_gridPoints;
	std::vector<double>							m_gridAreas; // Later

	IBKMK::Polygon3D							m_polygon;
	std::vector<IBKMK::Vector2D>				m_projectedPoly;
	std::vector<IBKMK::Polygon2D>				m_holes;
	std::vector<std::vector<IBKMK::Vector2D>>	m_projectedHoles;

	double										m_minX;
	double										m_maxX;
	double										m_minY;
	double										m_maxY;

	unsigned int								m_id;
	unsigned int								m_idParent;
	std::string									m_name;
	IBKMK::Vector3D								m_normal;

	std::ofstream								*m_outputFile = nullptr;
};

} // namespace SH


#endif // SH_ShadedSurfaceObjectH
