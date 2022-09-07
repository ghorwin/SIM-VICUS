#include "RC_Project.h"

#include <IBKMK_3DCalculations.h>
#include <VICUS_Object.h>

#include "RC_ClippingSurface.h"

#include <clipper.hpp>

void RC::Project::findParallelSurfaces() {


	std::set<VICUS::Surface*>	surfaces;

	for(const VICUS::Building &b : m_prjVicus.m_buildings) {
		for(const VICUS::BuildingLevel &bl : b.m_buildingLevels) {
			for(const VICUS::Room &r : bl.m_rooms) {
				for(const VICUS::Surface &s : r.m_surfaces) {
					surfaces.insert(const_cast<VICUS::Surface*>(&s));

				}
			}
		}
	}

	for(const VICUS::Surface *s1 : surfaces){
		for(const VICUS::Surface *s2 : surfaces){

			// skip same surfaces
			if(s1 == s2)
				continue;

			const VICUS::Surface &surf1 = *s1;
			const VICUS::Surface &surf2 = *s2;

			if(surf1.m_parent->m_id == surf2.m_parent->m_id)
				continue; // only surfaces of different rooms are clipped

			// skip already handled surfaces
			if(m_connections.find(surf1.m_id) != m_connections.end())
				if(m_connections[surf1.m_id].find(surf2.m_id) != m_connections[surf1.m_id].end())
					continue;

			// calculation of normal deviation
			double angle = IBKMK::angleBetweenVectorsDeg(-1 * surf1.geometry().normal(), surf2.geometry().normal());

			// check if deviation is inside limits
			if(angle > m_normalDeviationInDeg)
				continue;


			// save parallel surfaces
			ClippingSurface &cs = getClippingSurfaceById(surf1.m_id);
			cs.m_clippingObjects.push_back(ClippingObject(surf2.m_id, surf2, 999) );

			ClippingSurface &cs2 = getClippingSurfaceById(surf2.m_id);
			cs2.m_clippingObjects.push_back(ClippingObject(surf1.m_id, surf1, 999) );

			qDebug() << s1->m_id << ": " << s1->m_displayName << " | " << s2->m_id << ": "<< s2->m_displayName ;

			m_connections[surf1.m_id].insert(surf2.m_id);
			m_connections[surf2.m_id].insert(surf1.m_id);
		}
	}
}

void RC::Project::findSurfacesInRange() {
	for(std::map<unsigned int, std::set<unsigned int>>::iterator	it = m_connections.begin();
																	it != m_connections.end();
																	++it){
		// look for clipping surface
		ClippingSurface &cs = getClippingSurfaceById(it->first);
		const VICUS::Surface &s1 = cs.m_vicusSurface;

		std::vector<ClippingObject> newClippingObjects;
		for(unsigned int idSurf2 : it->second){

			// get our co object
			unsigned int idx2 = 0 ;
			for(;idx2<cs.m_clippingObjects.size(); ++idx2) {
				if(idSurf2 == cs.m_clippingObjects[idx2].m_vicusId)
					break;
			}
			ClippingObject &co = cs.m_clippingObjects[idx2];
			const VICUS::Surface &s2 = co.m_vicusSurface;

			// clipping object is for normalized directional vectors equal the distance of the two points
			IBKMK::Vector3D rayEndPoint;
			IBKMK::lineToPointDistance( s1.geometry().offset(), s1.geometry().normal().normalized(),
										s2.geometry().offset(), co.m_distance, rayEndPoint);
			if(co.m_distance > m_maxDistanceOfSurfaces || co.m_distance < 0)
				continue;

			newClippingObjects.push_back(co);
		}
		std::sort(newClippingObjects.begin(), newClippingObjects.end());
		// swap old clipping objects with newly sorted and in range surfaces
		cs.m_clippingObjects.swap(newClippingObjects);
	}
}

void RC::Project::clipSurfaces() {

	unsigned int id = m_newPrjVicus.nextUnusedID();

	for(std::map<unsigned int, std::set<unsigned int>>::iterator	it = m_connections.begin();
																	it != m_connections.end();
																	++it){

		// look for clipping surface
		ClippingSurface &cs = getClippingSurfaceById(it->first);
		VICUS::Surface s1 = cs.m_vicusSurface;

		IBKMK::Vector3D localX = s1.geometry().localX();
		IBKMK::Vector3D localY = s1.geometry().localY();
		IBKMK::Vector3D offset = s1.geometry().offset();
		QString displayName = s1.m_displayName;

		VICUS::Room *r = m_newPrjVicus.roomByID(s1.m_parent->m_id);
		unsigned int surfIdx = 0;

		// init all cutting objects
		std::vector<IBKMK::Polygon2D> mainDiffs, mainIntersections, clippingPolygons;
		clippingPolygons.push_back(s1.geometry().polygon2D());

		if(cs.m_clippingObjects.empty())
			continue;

		// delete original surfaces
		unsigned int eraseIdx = 0;
		for(;eraseIdx<r->m_surfaces.size(); ++eraseIdx){
			if(r->m_surfaces[eraseIdx].m_id == s1.m_id)
				break;
		}
		r->m_surfaces.erase(r->m_surfaces.begin()+eraseIdx);
		r->updateParents();

		for(ClippingObject co : cs.m_clippingObjects){
			const VICUS::Surface &s2 = co.m_vicusSurface;

			IBKMK::Polygon2D hole;

			// calculate new projection points onto our main polygon plane (clipper works 2D)
			std::vector<IBKMK::Vector2D> vertexes(s2.geometry().polygon2D().vertexes().size());
			qDebug() << s2.m_displayName << " | " << s1.m_displayName;
			for(unsigned int i=0; i<vertexes.size(); ++i){

				if(s2.geometry().polygon3D().vertexes().empty())
					continue;

				const IBKMK::Vector3D &p = s2.geometry().polygon3D().vertexes()[i];
				qDebug() << "x: " << p.m_x << "y: " << p.m_y << "z: " << p.m_z;
				// project points onto the plane
				try {
					IBKMK::planeCoordinates(s1.geometry().offset(), localX, localY,
											p-co.m_distance*s1.geometry().normal(), vertexes[i].m_x, vertexes[i].m_y);

				}  catch (...) {
					continue;
				}
			}

			doClipperClipping(clippingPolygons.back(), vertexes, mainDiffs, mainIntersections, hole);
			clippingPolygons.pop_back();

			// do clipping with clipper lib

			for(IBKMK::Polygon2D &poly : mainIntersections) {

				if(!poly.isValid())
					continue;

				s1.m_id = ++id;
				s1.m_displayName = QString("%2 [%1]").arg(++surfIdx).arg(displayName);

				// calculate new offset 3D
				IBKMK::Vector3D newOffset3D = offset	+ localX * poly.vertexes()[0].m_x
														+ localY * poly.vertexes()[0].m_y;
				// calculate new ofsset 2D
				IBKMK::Vector2D newOffset2D = poly.vertexes()[0];

				// move our points
				for(const IBKMK::Vector2D &v : poly.vertexes())
					const_cast<IBKMK::Vector2D &>(v) -= newOffset2D;

				// update VICUS Surface with new geometry
				const_cast<IBKMK::Polygon2D&>(s1.geometry().polygon2D()).setVertexes(poly.vertexes());
				IBKMK::Polygon3D poly3D = s1.geometry().polygon3D();
				poly3D.setTranslation(newOffset3D);
				s1.setPolygon3D(poly3D);		// now marked dirty = true

				r->m_surfaces.push_back(s1);
				r->updateParents();
			}

			// check diff for valid ...

			for(IBKMK::Polygon2D &diffPoly : mainDiffs){
				clippingPolygons.push_back(diffPoly);
			}
		}

		// push back polygon rests
		for(IBKMK::Polygon2D &poly : mainDiffs) {

			if(!poly.isValid())
				continue;

			s1.m_id = ++id;
			s1.m_displayName = QString("%2 [%1]").arg(++surfIdx).arg(displayName);

			// calculate new offset 3D
			IBKMK::Vector3D newOffset3D = offset	+ localX * poly.vertexes()[0].m_x
													+ localY * poly.vertexes()[0].m_y;
			// calculate new ofsset 2D
			IBKMK::Vector2D newOffset2D = poly.vertexes()[0];

			// move our points
			for(const IBKMK::Vector2D &v : poly.vertexes())
				const_cast<IBKMK::Vector2D &>(v) -= newOffset2D;

			// update VICUS Surface with new geometry
			const_cast<IBKMK::Polygon2D&>(s1.geometry().polygon2D()).setVertexes(poly.vertexes());
			IBKMK::Polygon3D poly3D = s1.geometry().polygon3D();
			poly3D.setTranslation(newOffset3D);
			s1.setPolygon3D(poly3D);		// now marked dirty = true

			r->m_surfaces.push_back(s1);
			r->updateParents();
		}

	}
}

RC::ClippingSurface& RC::Project::getClippingSurfaceById(unsigned int id) {
	// look for clipping surface
	unsigned int idx = 0 ;
	bool found = false;
	for(;idx<m_clippingSurfaces.size(); ++idx) {
		if(id == m_clippingSurfaces[idx].m_vicusId) {
			found = true;
			break;
		}
	}
	if (!found) {
		const VICUS::Surface *surf = dynamic_cast<const VICUS::Surface*>(m_prjVicus.objectById(id));
		Q_ASSERT(surf != nullptr);
		m_clippingSurfaces.push_back(ClippingSurface(id, *surf));
		return m_clippingSurfaces.back();
	}

	return m_clippingSurfaces[idx];
}

void RC::Project::doClipperClipping(const IBKMK::Polygon2D &surf,
									const IBKMK::Polygon2D &otherSurf,
									std::vector<IBKMK::Polygon2D> &mainDiffs,
									std::vector<IBKMK::Polygon2D> &mainIntersections,
									IBKMK::Polygon2D &hole,
									bool normalInterpolation) {

	ClipperLib::Paths	mainPoly(1);
	ClipperLib::Path	&polyClp = mainPoly.back();

	// set up first polygon for clipper
	for(const IBKMK::Vector2D &p : surf.vertexes())
		polyClp << ClipperLib::IntPoint(static_cast<long long>(p.m_x * SCALE_FACTOR),
										static_cast<long long>(p.m_y * SCALE_FACTOR));

	// set up clipper lib paths
	ClipperLib::Paths	otherPolys(1);
	IBKMK::Polygon2D poly2D(otherSurf.vertexes());

	// set up second polygon for clipper
	for(const IBKMK::Vector2D &p : otherSurf.vertexes())
		otherPolys.back() << ClipperLib::IntPoint(	static_cast<long long>(p.m_x * SCALE_FACTOR),
													static_cast<long long>(p.m_y * SCALE_FACTOR));

	// init clipper object
	ClipperLib::Clipper clp;

	// add clipper lib paths with geometry from surfaces
	clp.AddPaths(mainPoly, ClipperLib::ptSubject, true);
	clp.AddPaths(otherPolys, ClipperLib::ptClip, true);

	// do finally all CLIPPINGS in CLIPPER LIB
	ClipperLib::Paths solutionIntersection, solutionDiff;
	clp.Execute(ClipperLib::ctIntersection, solutionIntersection, ClipperLib::pftEvenOdd, ClipperLib::pftEvenOdd);
	clp.Execute(ClipperLib::ctDifference, solutionDiff, ClipperLib::pftEvenOdd, ClipperLib::pftEvenOdd);

	// Convert all geometries back to our data structure
	mainDiffs = std::vector<IBKMK::Polygon2D>(solutionDiff.size());
	mainIntersections = std::vector<IBKMK::Polygon2D>(solutionIntersection.size());

	// convert all diff polygons
	for(unsigned int i=0; i<solutionDiff.size(); ++i) {

		const ClipperLib::Path &path = solutionDiff[i];
		IBKMK::Polygon2D &poly = mainDiffs[i];

		std::vector<IBKMK::Vector2D> vert2D;
		for(const ClipperLib::IntPoint &ip : path) {
			vert2D.push_back(IBKMK::Vector2D((double)ip.X/SCALE_FACTOR, (double)ip.Y/SCALE_FACTOR));
		}
		poly.setVertexes(vert2D);
	}

	// convert all interscetion polygons
	for(unsigned int i=0; i<solutionIntersection.size(); ++i) {

		const ClipperLib::Path &path = solutionIntersection[i];
		IBKMK::Polygon2D &poly = mainIntersections[i];

		std::vector<IBKMK::Vector2D> vert2D;
		for(const ClipperLib::IntPoint &ip : path) {
			vert2D.push_back(IBKMK::Vector2D((double)ip.X/SCALE_FACTOR, (double)ip.Y/SCALE_FACTOR));
		}
		poly.setVertexes(vert2D);
	}


}

const VICUS::Project &RC::Project::newPrjVicus() const {
	return m_newPrjVicus;
}

