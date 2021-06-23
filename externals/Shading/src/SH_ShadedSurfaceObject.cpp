#include "SH_ShadedSurfaceObject.h"

#include <IBK_Exception.h>

namespace SH {

void ShadedSurfaceObject::setPolygon(const Polygon & surface, double gridWidth) {
	FUNCID(ShadedSurfaceObject::setPolygon);

	IBK_ASSERT(gridWidth > 0);

	//rotate polygon to z-plane
	//get normal of polygon
	IBKMK::Vector3D n = surface.calcNormal();
	IBKMK::Vector3D zAxis(0,0,1);

	double cosAlpha = n.scalarProduct(zAxis);
	//überlegen wenn cosAlpha 0 dann keine Rotation, 180° Rotation auch nicht nötig
	IBKMK::Vector3D rotaAxis = n.crossProduct(zAxis);

	Polygon poly(surface); // create copy of polygon, to be rotated into xy-plane

	//nur wenn ein Vector entsteht muss gedreht werden, bei Null-Vector keine Drehung nötig
	IBK::matrix<double> rotaMatrix(3,3,0), backRotaMatrix(3,3,0);
	if(rotaAxis.magnitude()>0.1){
		//rotationsmatrix
		rotaMatrix = Polygon::rotationMatrixForStraightOrigin(cosAlpha, rotaAxis,true);
		backRotaMatrix = Polygon::rotationMatrixForStraightOrigin(cosAlpha, rotaAxis, false);

		//m_polyline rotieren
		poly.rotatePolyline(rotaMatrix);
	}

	// 1. create a grid for the polygon

	//find min and max in x and y direction
	double minX = std::numeric_limits<double>::max();
	double minY = std::numeric_limits<double>::max();
	double maxX = std::numeric_limits<double>::min();
	double maxY = std::numeric_limits<double>::min();
	for (size_t i=0; i<poly.m_polyline.size(); ++i) {
		const IBKMK::Vector3D &p = poly.m_polyline[i];
		minX = std::min(minX, p.m_x);
		minY = std::min(minY, p.m_y);
		maxX = std::max(maxX, p.m_x);
		maxY = std::max(maxY, p.m_y);
	}

	double distanceX = maxX - minX;
	double distanceY = maxY - minY;

	for (size_t xSteps=0, xM=std::ceil(distanceX/gridWidth); xSteps<xM; ++xSteps) {
		for (size_t ySteps=0; ySteps<std::ceil(distanceY/gridWidth); ++ySteps) {
			IBKMK::Vector3D newMiddlePoint(minX + (xSteps + 0.5)*gridWidth, minY + (ySteps + 0.5)*gridWidth, poly.m_polyline[0].m_z);
			if(poly.pointInPolygon(newMiddlePoint) >= 0) {
				// rotate grid point back to 3D space
				m_gridPoints.push_back(newMiddlePoint);
			}
		}
	}

	//if no middlepoint was good enough
	if(m_gridPoints.empty())
		m_gridPoints.insert(m_gridPoints.begin(), poly.m_polyline.begin(), poly.m_polyline.end());

	poly = Polygon (m_gridPoints);
	poly.rotatePolyline(backRotaMatrix);

	for (unsigned int i=0; i<poly.m_polyline.size(); ++i)
		m_gridPoints[i] = poly.m_polyline[i];
}


double ShadedSurfaceObject::calcShadingFactor(const IBKMK::Vector3D &sunNormal, const std::vector<Polygon> & m_obstacles) const {
	unsigned int counterShadedPoints=0;

	unsigned int sizeMiddlePoints = m_gridPoints.size();

	// process all obstacles
	for (size_t j=0; j<m_obstacles.size(); ++j) {
		IBKMK::Vector3D nPlane = m_obstacles[j].calcNormal();
		Polygon otherPoly(m_obstacles[j].m_polyline);

		double d = nPlane.scalarProduct(m_obstacles[j].m_polyline[0]);

		// process all grid points
		for (size_t i=0; i<sizeMiddlePoints; ++i) {

			// point of the plane
			const IBKMK::Vector3D &mP = m_gridPoints[i];

			// TODO Stephan, formel raussuchen
			double lambda = (d - (nPlane.m_x * mP.m_x + nPlane.m_y * mP.m_y + nPlane.m_z * mP.m_z)) /
					(nPlane.m_x * sunNormal.m_x + nPlane.m_y * sunNormal.m_y + nPlane.m_z * sunNormal.m_z);
			// wrong direction
			if (lambda<=0)
				continue;

			IBKMK::Vector3D obstacleIntersectionPlanePoint(mP.m_x + lambda * sunNormal.m_x, mP.m_y + lambda * sunNormal.m_y, mP.m_z + lambda * sunNormal.m_z);

			//hit obstacle
			//int testDirk = otherPoly.pointInPolygon(obstacleIntersectionPlanePoint);
			if (otherPoly.pointInPolygon(obstacleIntersectionPlanePoint)!=-1)
				++counterShadedPoints; // increase shaded grid point count
		}
	}

	double sf = double(counterShadedPoints)/sizeMiddlePoints;
	return sf;
}


} // namespace SH

