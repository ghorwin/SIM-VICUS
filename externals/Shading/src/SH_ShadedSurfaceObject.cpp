#include "SH_ShadedSurfaceObject.h"

#include <IBK_Exception.h>
#include <IBK_assert.h>

#include <IBKMK_2DCalculations.h>
#include <IBKMK_3DCalculations.h>

#include <clipper.hpp>

#include <QDebug>

#include <fstream>

namespace SH {

ClipperLib::IntPoint convertVector2D2ClipperIntPoint(const IBKMK::Vector2D &v2D) {
	return ClipperLib::IntPoint((ClipperLib::cInt)((double)SCALE_FACTOR*v2D.m_x),
								(ClipperLib::cInt)((double)SCALE_FACTOR*v2D.m_y) );
}

void ShadedSurfaceObject::setPolygon(unsigned int id, std::string name, const IBKMK::Polygon3D & surface, const std::vector<IBKMK::Polygon2D> &holes,
									 unsigned int idParent, double gridWidth, bool useClipping) {
	IBK_ASSERT(gridWidth > 0);

	m_id = id;
	m_idParent = idParent;
	m_name = name;
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

			if (m_id == obstacles[j].m_idVicus)
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

double ShadedSurfaceObject::calcShadingFactorWithClipping(unsigned int idxSun, const IBKMK::Vector3D &sunNormal,
														  const std::vector<StructuralShading::ShadingObject> & obstacles) const {
	// process all obstacles

#ifdef WRITE_OUTPUT
	*m_outputFile << std::endl;
	*m_outputFile << std::endl;
	*m_outputFile << "****************************" << std::endl;
	*m_outputFile << "Surface: " << m_name << std::endl;
#endif

	ClipperLib::Paths clpPolygon;
	// To be eventually shaded poly will be added
	ClipperLib::Path pathPolygon;
	for(const IBKMK::Vector2D v2D : m_projectedPoly)
		pathPolygon << convertVector2D2ClipperIntPoint(v2D);
	clpPolygon << pathPolygon;

#ifdef WRITE_OUTPUT
	writePathToOutputFile("Path Point: ", pathPolygon);
#endif

	bool polyOrientation = ClipperLib::Orientation(pathPolygon);

	// Surface area of original polygon
	double surfaceArea = 0;
	surfaceArea = ClipperLib::Area(pathPolygon);

	// All the holes will be added
	for(const std::vector<IBKMK::Vector2D> &hole : m_projectedHoles) {

		ClipperLib::Path pathHole;

		for(const IBKMK::Vector2D v2D : hole)
			pathHole << convertVector2D2ClipperIntPoint(v2D);

#ifdef WRITE_OUTPUT
		*m_outputFile << "------------------------------" << std::endl;
		writePathToOutputFile("PathHole Point: ", pathHole);
#endif

		// MIND: Hole needs different orientation then subject poly path
		if (polyOrientation == ClipperLib::Orientation(pathHole) )
			ClipperLib::ReversePath(pathHole);

		clpPolygon << pathHole;

		surfaceArea += ClipperLib::Area(pathHole);
	}

	ClipperLib::Clipper clp;
	clp.AddPaths(clpPolygon, ClipperLib::ptSubject, true);

#ifdef WRITE_OUTPUT
	*m_outputFile << "------------------------------" << std::endl;
#endif

	ClipperLib::Paths clpObstacles;
	// All obstacles will be added to clipper
	for (size_t j=0; j<obstacles.size(); ++j) {

		ClipperLib::Path pathObstacle;
		for(const IBKMK::Vector2D &v2D : obstacles[j].m_projectedPolys[idxSun])
			pathObstacle << convertVector2D2ClipperIntPoint(v2D);

		bool obstacleOrientation = ClipperLib::Orientation(pathObstacle);
		if (polyOrientation != obstacleOrientation)
			ClipperLib::ReversePath(pathObstacle);

		clpObstacles << pathObstacle;

#ifdef WRITE_OUTPUT
		*m_outputFile << "------------------------------" << std::endl;
		writePathToOutputFile("Obstacle Point: ", pathObstacle);
		// *m_outputFile << "Parent ID of Surface: " << m_idParent << " | " << INVALID_ID;
		// *m_outputFile << "Obstacle ID: " <<  obstacles[j].m_id;
#endif


		if(m_idParent == INVALID_ID || m_idParent != obstacles[j].m_idVicus)
			continue;

		for(const std::vector<IBKMK::Vector2D> &hole : obstacles[j].m_projectedHoles[idxSun] ) {


			ClipperLib::Path pathHole;

			for(const IBKMK::Vector2D &v2D : hole)
				pathHole << convertVector2D2ClipperIntPoint(v2D);

#ifdef WRITE_OUTPUT
			*m_outputFile << "------------------------------" << std::endl;
			writePathToOutputFile("ObstacleHole Point: ", pathHole);
#endif

			// MIND: Hole needs different orientation then subject poly path
			if (ClipperLib::Orientation(pathObstacle) == ClipperLib::Orientation(pathHole) )
				ClipperLib::ReversePath(pathHole);

			clpObstacles << pathHole; // Add hole
		}
	}
	clp.AddPaths(clpObstacles, ClipperLib::ptClip, true);

	// Do all the cutting with our eventually shaded polygon
	ClipperLib::PolyTree diffPaths;
	clp.Execute(ClipperLib::ctDifference, diffPaths, ClipperLib::pftNonZero, ClipperLib::pftNonZero);

	// If we found no diffs, we do not have any sunlight areas
	if(diffPaths.Total() == 0) // No diffs so far an thus no lighting
		return 0;


	// We go through our Polytree hierarchy and add all areas
	// Hole should have negative areas
	double clippingArea = 0;
	addAreaOfPolyNode(diffPaths.GetNext(), clippingArea);

	// Return the shading factor
	return clippingArea / surfaceArea;
}

void ShadedSurfaceObject::setProjectedPolygonAndHoles(const std::vector<IBKMK::Vector2D> & poly,
													  const std::vector<std::vector<IBKMK::Vector2D>> & holes) {
	m_projectedPoly = poly;
	m_projectedHoles = holes;
}

void ShadedSurfaceObject::addAreaOfPolyNode(const ClipperLib::PolyNode * polyNode, double & area) const {
	if(polyNode == nullptr)
		return;

#ifdef WRITE_OUTPUT
	*m_outputFile << "-----------------------------" << std::endl;
	writePathToOutputFile("Intersection Point: ", polyNode->Contour);
#endif

	area += ClipperLib::Area(polyNode->Contour);
	addAreaOfPolyNode(polyNode->GetNext(), area);
}

void ShadedSurfaceObject::writePathToOutputFile(const std::string preText, const ClipperLib::Path & path) const {
	for(const ClipperLib::IntPoint &ip : path)
		*m_outputFile << preText << ip.X << " " << ip.Y << std::endl;
}

void ShadedSurfaceObject::setOutputFile(std::ofstream * newOutputFile) {
	m_outputFile = newOutputFile;
}


} // namespace SH

