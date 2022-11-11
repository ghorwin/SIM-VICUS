#include "SH_ShadedSurfaceObject.h"

#include <IBK_Exception.h>
#include <IBK_assert.h>

#include <IBKMK_2DCalculations.h>
#include <IBKMK_3DCalculations.h>

#include <clipper.hpp>

#include <QDebug>

namespace SH {

void ShadedSurfaceObject::setPolygon(unsigned int id, const IBKMK::Polygon3D & surface,
									 const std::vector<IBKMK::Polygon2D> &holes, double gridWidth, bool useClipping) {
	IBK_ASSERT(gridWidth > 0);

	m_id = id;
	m_normal = surface.normal();
	m_polygon = surface;
	m_holes = holes;

	if(useClipping)
		return;

	//  ONLY FOR RAY-TRACING NEEDED ====================================

	//find min and max in x and y direction
	m_minX = std::numeric_limits<double>::max();
	m_minY = std::numeric_limits<double>::max();
	m_maxX = std::numeric_limits<double>::min();
	m_maxY = std::numeric_limits<double>::min();
	const std::vector<IBKMK::Vector2D> & polyline = surface.polyline().vertexes();
	for (size_t i=0; i<polyline.size(); ++i) {
		const IBKMK::Vector2D &p = polyline[i];
		m_minX = std::min(m_minX, p.m_x);
		m_minY = std::min(m_minY, p.m_y);
		m_maxX = std::max(m_maxX, p.m_x);
		m_maxY = std::max(m_maxY, p.m_y);
	}

	double distanceX = m_maxX - m_minX;
	double distanceY = m_maxY - m_minY;

	for (size_t xSteps=0, xM=(size_t)std::ceil(distanceX/gridWidth); xSteps<xM; ++xSteps) {
		for (size_t ySteps=0; ySteps<std::ceil(distanceY/gridWidth); ++ySteps) {
			bool isHolePoint = false;
			// determine grid point in grid
			IBKMK::Vector2D newMiddlePoint(m_minX + (xSteps + 0.5)*gridWidth, m_minY + (ySteps + 0.5)*gridWidth);
			// check if grid point is inside a hole
			for(const IBKMK::Polygon2D &holePoly : holes) {
				if(IBKMK::pointInPolygon(holePoly.vertexes(), newMiddlePoint) >= 0) {
					isHolePoint = true;
					break;
				}
			}
			if(isHolePoint)
				continue;

			// only store grid point if it is inside the polygon
			if (IBKMK::pointInPolygon(polyline, newMiddlePoint) >= 0) {
				// store original point
				m_gridPoints.push_back( surface.vertexes()[0] + surface.localX() * newMiddlePoint.m_x + surface.localY() * newMiddlePoint.m_y);
			}
		}
	}

	//if no middlepoint was good enough, just check all boundary points
	if (m_gridPoints.empty())
		m_gridPoints = surface.vertexes();
}


double ShadedSurfaceObject::calcShadingFactorWithRayTracing(const IBKMK::Vector3D &sunNormal, const std::vector<StructuralShading::ShadingObject> & obstacles) const {
	unsigned int counterShadedPoints=0;

	unsigned int sizeMiddlePoints = m_gridPoints.size();
	// process all grid points
	for (size_t i=0; i<sizeMiddlePoints; ++i) {

		// process all obstacles
		for (size_t j=0; j<obstacles.size(); ++j) {

			if (m_id == obstacles[j].m_id)
				continue;

			// compute intersection point of sun beam onto obstacle's plane
			const IBKMK::Vector3D & offset = obstacles[j].m_polygon.vertexes()[0];
			IBKMK::Vector3D intersectionPoint;
			double dist;
			if (!IBKMK::linePlaneIntersectionWithNormalCheck(offset, obstacles[j].m_polygon.normal(), // plane
											  m_gridPoints[i], sunNormal, // line
											  intersectionPoint, dist, !obstacles[j].m_isObstacle))
				continue; // no intersection, next obstacle plane

			// compute local coordinates of intersection point with obstacle
			double x,y;
			if (!IBKMK::planeCoordinates(offset, obstacles[j].m_polygon.localX(), obstacles[j].m_polygon.localY(), intersectionPoint, x, y))
				continue; // projection not possible - this shouldn't happen, really!

			// now test if x,y coordinates are inside obstacle's polyline
			if (IBKMK::pointInPolygon(obstacles[j].m_polygon.polyline().vertexes(), IBK::point2D<double>(x,y)) >= 0) {
				++counterShadedPoints;
				break; // we are shaded, stop searching
			}
		}
	}

	double sf = 1 - double(counterShadedPoints)/sizeMiddlePoints;
	return sf;
}

double ShadedSurfaceObject::calcShadingFactorWithClipping(unsigned int idxSun, const IBKMK::Vector3D &sunNormal, const std::vector<StructuralShading::ShadingObject> & obstacles) const {

	if(IBKMK::angleBetweenVectorsDeg(sunNormal, m_polygon.normal()) > 90)
		return 0;

	// process all obstacles
	ClipperLib::Clipper clp;

	ClipperLib::PolyTree polyTree;
	ClipperLib::Paths clpPolygon;
	ClipperLib::Path pathPolygon;

	for(const IBKMK::Vector2D v2D : m_projectedPoly) {
		pathPolygon << ClipperLib::IntPoint(SCALE_FACTOR*v2D.m_x, SCALE_FACTOR*v2D.m_y);
	}
	clpPolygon << pathPolygon;

	bool polyOrientation = ClipperLib::Orientation(pathPolygon);

	for(const std::vector<IBKMK::Vector2D> &hole : m_projectedHoles) {
		ClipperLib::Path pathHole;
		for(const IBKMK::Vector2D v2D : hole) {
			pathHole << ClipperLib::IntPoint(SCALE_FACTOR*v2D.m_x, SCALE_FACTOR*v2D.m_y);
		}

		// MIND: Hole needs different orientation then subject poly path
		if (polyOrientation == ClipperLib::Orientation(pathHole) )
			ClipperLib::ReversePath(pathHole);

		clpPolygon << pathHole;
	}

	clp.AddPaths(clpPolygon, ClipperLib::ptSubject, true);

	qDebug() << "Add Obstacles";

	ClipperLib::Clipper clpUnion;
	// project in poly pane
	for (size_t j=0; j<obstacles.size(); ++j) {


		if (m_id == obstacles[j].m_id)
			continue;

		ClipperLib::Path pathObstacle;
		for(const IBKMK::Vector2D &v2D : obstacles[j].m_projectedPolys[idxSun]) {
			pathObstacle << ClipperLib::IntPoint(SCALE_FACTOR*v2D.m_x, SCALE_FACTOR*v2D.m_y);
		}

		clpUnion.AddPath(pathObstacle, ClipperLib::ptSubject, true);
	}

	ClipperLib::Paths obstaclePaths;
	clpUnion.Execute(ClipperLib::ctUnion, obstaclePaths, ClipperLib::pftEvenOdd, ClipperLib::pftEvenOdd);
	clp.AddPaths(obstaclePaths, ClipperLib::ptClip, true);

	qDebug() << "///////////////////";
	for(const ClipperLib::Path &pathObstacle : obstaclePaths) {
		qDebug() << "xxxxxxxxxxxxxxxxxxxxx";
		for(const ClipperLib::IntPoint &ip : pathObstacle)
			qDebug() << "Obstacle Point: " << ip.X << " " << ip.Y;
	}
	qDebug() << "///////////////////";


	ClipperLib::Paths intersectionPaths;
	clp.Execute(ClipperLib::ctIntersection, intersectionPaths, ClipperLib::pftEvenOdd, ClipperLib::pftEvenOdd);

	if(intersectionPaths.empty())
		return 1;

	double surfaceArea = ClipperLib::Area(pathPolygon);

	double clippingArea = 0;
	if(intersectionPaths.size() > 1) {

		for(const ClipperLib::IntPoint &ip : pathPolygon)
			qDebug() << "Path Point: " << ip.X << " " << ip.Y;


		qDebug() << "============================";
		for(const ClipperLib::Path &path : intersectionPaths) {
			for(const ClipperLib::IntPoint &ip : path)
				qDebug() << "Intersect Point: " << ip.X << " " << ip.Y;
			qDebug() << "------------------------------";
			clippingArea += std::abs(ClipperLib::Area(path));
		}
	}
	else
		clippingArea = std::abs(ClipperLib::Area(intersectionPaths[0]));


	return (1.0 - clippingArea / surfaceArea);
}

void ShadedSurfaceObject::setProjectedPolygonAndHoles(const std::vector<IBKMK::Vector2D> & poly,
													  const std::vector<std::vector<IBKMK::Vector2D>> & holes) {
	m_projectedPoly = poly;
	m_projectedHoles = holes;
}


} // namespace SH

