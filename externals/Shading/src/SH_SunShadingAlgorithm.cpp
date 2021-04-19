#include "SH_SunShadingAlgorithm.h"

#include <IBK_Exception.h>

namespace SH {

void SunShadingAlgorithm::calcShading(const IBKMK::Vector3D & sunNormal) {
	for (size_t i=0; i<m_shadingObjects.size(); ++i)
		calcShadingOneElement( sunNormal);
}

void SunShadingAlgorithm::initializeGrid() {

	FUNCID(SunShadingAlgorithm::initializeGrid);

	for ( size_t i=0; i<m_shadingObjects.size(); ++i) {

		SunShadingAlgorithm::ShadingObj &obj = m_shadingObjects[i];

		if(obj.m_polygon.m_polyline.size() < 3)
			throw IBK::Exception(IBK::FormatString("Polyline is not valid."), FUNC_ID);

		//new Polygon
		Polygon poly(obj.m_polygon);
		//rotate polygon to z-plane
		//get normal of polygon
		IBKMK::Vector3D n = poly.calcNormal();
		IBKMK::Vector3D zAxis(0,0,1);

		double cosAlpha = n.scalarProduct(zAxis);
		//überlegen wenn cosAlpha 0 dann keine Rotation, 180° Rotation auch nicht nötig
		IBKMK::Vector3D rotaAxis = n.crossProduct(zAxis);

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

		std::vector<IBKMK::Vector3D>	gridMiddlePoints;

		for (size_t xSteps=0, xM=std::ceil(distanceX/m_gridLength); xSteps<xM; ++xSteps) {
			for (size_t ySteps=0; ySteps<std::ceil(distanceY/m_gridLength); ++ySteps) {
				IBKMK::Vector3D newMiddlePoint(minX + (xSteps + 0.5)*m_gridLength, minY + (ySteps + 0.5)*m_gridLength, poly.m_polyline[0].m_z);
				if(poly.pointInPolygon(newMiddlePoint) >= 0)
					gridMiddlePoints.push_back(newMiddlePoint);
			}
		}

		//if no middlepoint was good enough
		if(gridMiddlePoints.empty())
			gridMiddlePoints.insert(gridMiddlePoints.begin(), poly.m_polyline.begin(), poly.m_polyline.end());

		obj.m_grid = Polygon (gridMiddlePoints);
		obj.m_grid.rotatePolyline(backRotaMatrix);

		obj.m_isGridInitialized = true;
	}
}

void SunShadingAlgorithm::calcShadingOneElement(const IBKMK::Vector3D &sunNormal) {

	for (size_t i=0; i<m_shadingObjects.size() ; ++i) {

		SunShadingAlgorithm::ShadingObj obj = m_shadingObjects[i];	// make a copy because modifing of points
		unsigned int counterShadedPoints=0;
		unsigned int sizeMiddlePoints = obj.m_grid.m_polyline.size();
		// 2. shading algorithm
		for (size_t j=0; j<m_obstacles.size(); ++j) {
			IBKMK::Vector3D nPlane = m_obstacles[j].calcNormal();
			Polygon otherPoly(m_obstacles[j].m_polyline);

			double d = nPlane.scalarProduct(m_obstacles[j].m_polyline[0]);
			//cutting point plane and straight line

			std::vector<unsigned int>	delPos;
			for (size_t i=0; i<obj.m_grid.m_polyline.size(); ++i) {
				//point of the plane
				const IBKMK::Vector3D &mP = obj.m_grid.m_polyline[i];
				double lambda = (d - (nPlane.m_x * mP.m_x + nPlane.m_y * mP.m_y + nPlane.m_z * mP.m_z)) /
						(nPlane.m_x * sunNormal.m_x + nPlane.m_y * sunNormal.m_y + nPlane.m_z * sunNormal.m_z);
				//wrong direction
				if(lambda<=0)
					continue;

				IBKMK::Vector3D obstacleIntersectionPlanePoint(mP.m_x + lambda * sunNormal.m_x, mP.m_y + lambda * sunNormal.m_y, mP.m_z + lambda * sunNormal.m_z);

				//hit obstacle
				//int testDirk = otherPoly.pointInPolygon(obstacleIntersectionPlanePoint);
				if(otherPoly.pointInPolygon(obstacleIntersectionPlanePoint)!=-1){
					++counterShadedPoints;
						delPos.push_back(i);
				}
			}

			//delete points which hit an obstacle
			while(!delPos.empty()){
				obj.m_grid.m_polyline.erase(obj.m_grid.m_polyline.begin()+delPos.back());
				delPos.pop_back();
			}
		}
		m_shadingObjects[i].m_shadingValue = (double)(sizeMiddlePoints - counterShadedPoints) / sizeMiddlePoints;
	}
}

} // namespace SH

