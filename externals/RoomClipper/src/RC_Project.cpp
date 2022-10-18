#include "RC_Project.h"

#include <IBKMK_3DCalculations.h>
#include <VICUS_Object.h>

#include "RC_ClippingSurface.h"


namespace RC {

void Project::findParallelSurfaces() {


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

void Project::findSurfacesInRange() {
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

void Project::clipSurfaces() {

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
		std::vector<extPolygon> mainDiffs, mainIntersections, clippingPolygons;
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

		for(ClippingObject &co : cs.m_clippingObjects){
			const VICUS::Surface &s2 = co.m_vicusSurface;

			IBKMK::Polygon2D hole;

			// calculate new projection points onto our main polygon plane (clipper works 2D)
			std::vector<IBKMK::Vector2D> vertexes(s2.geometry().polygon2D().vertexes().size());
			qDebug() << s2.m_displayName << " | " << s1.m_displayName;
			for(unsigned int i=0; i<vertexes.size(); ++i){

				if(s2.geometry().polygon3D().vertexes().empty())
					continue;

				const IBKMK::Vector3D &p = s2.geometry().polygon3D().vertexes()[i];
				qDebug() << "Point outside plane x: " << p.m_x << "y: " << p.m_y << "z: " << p.m_z;
				IBKMK::Vector3D pNew = p-co.m_distance*s1.geometry().normal();
				qDebug() << "Point inside plane x: " << pNew.m_x << "y: " << pNew.m_y << "z: " << pNew.m_z;
				// project points onto the plane
				try {
					IBKMK::planeCoordinates(offset, localX, localY,
											p-co.m_distance*s1.geometry().normal(), vertexes[i].m_x, vertexes[i].m_y);

				}  catch (...) {
					continue;
				}
			}

			for(const extPolygon &clippingPoly : clippingPolygons)
				qDebug() << "Anzahl der Punkte: " << clippingPoly.m_polygon.vertexes().size();

			qDebug() << "Es wird jetzt Fläche '" << s1.m_displayName << "' von Fläche '" << s2.m_displayName << "' geschnitten.";

			std::vector<extPolygon> mainDiffsTemp, mainIntersectionsTemp;
			unsigned int maxSize = clippingPolygons.size();
			for(unsigned int i=0; i<maxSize; ++i){
				// do clipping with clipper lib
				doClipperClipping(clippingPolygons.back(), extPolygon(vertexes), mainDiffsTemp, mainIntersectionsTemp);
				clippingPolygons.pop_back();
				mainDiffs.insert(mainDiffs.end(), mainDiffsTemp.begin(), mainDiffsTemp.end());

				qDebug() << "MainDiffs";

				for(const extPolygon &polyPrint : mainDiffs){
					qDebug() << "MainDiff";
					for(const IBKMK::Vector2D pPrint : polyPrint.m_polygon.vertexes())
						qDebug() << pPrint.m_x << " " << pPrint.m_y;
				}

				mainIntersections.insert(mainIntersections.end(), mainIntersectionsTemp.begin(), mainIntersectionsTemp.end());

				qDebug() << "Intersections";

				for(const extPolygon &polyPrint : mainIntersections){
					qDebug() << "Intersection";
					for(const IBKMK::Vector2D pPrint : polyPrint.m_polygon.vertexes())
						qDebug() << pPrint.m_x << " " << pPrint.m_y;
				}
				qDebug() << "cutting finished";
			}

			for(extPolygon &poly : mainIntersections) {

				if(!poly.m_polygon.isValid())
					continue;



				s1.m_id = ++id;
				s1.m_displayName = QString("%2 [%1]").arg(++surfIdx).arg(displayName);

				// calculate new offset 3D
				IBKMK::Vector3D newOffset3D = offset	+ localX * poly.m_polygon.vertexes()[0].m_x
						+ localY * poly.m_polygon.vertexes()[0].m_y;
				// calculate new ofsset 2D
				IBKMK::Vector2D newOffset2D = poly.m_polygon.vertexes()[0];

				// move our points
				for(const IBKMK::Vector2D &v : poly.m_polygon.vertexes())
					const_cast<IBKMK::Vector2D &>(v) -= newOffset2D;

				// update VICUS Surface with new geometry
				const_cast<IBKMK::Polygon2D&>(s1.geometry().polygon2D()).setVertexes(poly.m_polygon.vertexes());
				IBKMK::Polygon3D poly3D = s1.geometry().polygon3D();
				poly3D.setTranslation(newOffset3D);
				s1.setPolygon3D(poly3D);		// now marked dirty = true
				// ToDo Stephan: Fenster einfügen auf neuen Ursprung
				r->m_surfaces.push_back(s1);
				r->updateParents();
			}

			// check diff for valid ...

			std::vector<unsigned int>	erasePos;

			for(unsigned int idx = 0; idx<mainDiffs.size(); ++idx){
				extPolygon &diffPoly = mainDiffs[idx];
				if(diffPoly.m_polygon.vertexes().empty()){
					erasePos.insert(erasePos.begin(), idx);
					continue;
				}
				clippingPolygons.push_back(diffPoly);
			}

			for(unsigned int idx : erasePos)
				mainDiffs.erase(mainDiffs.begin() + idx);


			if(mainDiffs.empty())
				break;

			mainIntersections.clear();
			mainDiffs.clear();
		}

		// push back polygon rests
		for(extPolygon &poly : clippingPolygons) {

			if(!poly.m_polygon.isValid())
				continue;

			s1.m_id = ++id;
			s1.m_displayName = QString("%2 [%1]").arg(++surfIdx).arg(displayName);

			// calculate new offset 3D
			IBKMK::Vector3D newOffset3D = offset	+ localX * poly.m_polygon.vertexes()[0].m_x
					+ localY * poly.m_polygon.vertexes()[0].m_y;
			// calculate new ofsset 2D
			IBKMK::Vector2D newOffset2D = poly.m_polygon.vertexes()[0];

			// move our points
			for(const IBKMK::Vector2D &v : poly.m_polygon.vertexes())
				const_cast<IBKMK::Vector2D &>(v) -= newOffset2D;

			// update VICUS Surface with new geometry
			const_cast<IBKMK::Polygon2D&>(s1.geometry().polygon2D()).setVertexes(poly.m_polygon.vertexes());
			IBKMK::Polygon3D poly3D = s1.geometry().polygon3D();
			poly3D.setTranslation(newOffset3D);
			s1.setPolygon3D(poly3D);		// now marked dirty = true

			// ==========================
			// CRAZY HOLE ACTION INCOMING

			if(poly.m_haveRealHole && poly.m_holePolygons.size() > 0) {
				for(unsigned int i=0; i<poly.m_holePolygons.size(); ++i) {
					IBKMK::Polygon2D &holePoly = poly.m_holePolygons[i];

					std::vector<IBKMK::Vector2D> holePoints(holePoly.vertexes().size());
					for(unsigned int j=0; j<holePoly.vertexes().size(); ++j) {
						const IBKMK::Vector2D &v2d = holePoly.vertexes()[j];

						IBKMK::Vector3D holePoint3D = offset
								+ localX * v2d.m_x
								+ localY * v2d.m_y;

						IBKMK::planeCoordinates(newOffset3D, s1.geometry().localX(),
												s1.geometry().localY(), holePoint3D, holePoints[j].m_x, holePoints[j].m_y);
					}

					VICUS::SubSurface ss;
					ss.m_polygon2D.setVertexes(holePoints);
					ss.m_id = ++id;
					ss.m_displayName = QString("%1 Hole [%1]").arg(s1.m_displayName ).arg(i);
					ss.m_parent = &s1;

					std::vector<VICUS::SubSurface> subSurfs = s1.subSurfaces();
					subSurfs.push_back(ss);
					s1.setSubSurfaces(subSurfs);
				}
			}

			// ==========================

			// Add back holes to data structure
			r->m_surfaces.push_back(s1);
			r->updateParents();
		}
	}
}

ClippingSurface& RC::Project::getClippingSurfaceById(unsigned int id) {
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

void Project::generatePolyWithHole(const IBKMK::Polygon2D &polygon,
								   const std::vector<IBKMK::Polygon2D> &holes,
								   IBKMK::Polygon2D &newPolygon,
								   bool &realHole) {



	//first find minimum distance between all holes and the original polygon

	std::vector<IBKMK::Vector2D> polyP = polygon.vertexes();
	std::vector<IBKMK::Polygon2D> tempHoles = holes;
	struct distanceData{
		unsigned int	m_idxHole;
		unsigned int	m_idxPolyPoint;
		unsigned int	m_idxHolePoint;
		double			m_distance;
	};

	while(!tempHoles.empty()){
		distanceData dd;
		dd.m_distance = std::numeric_limits<double>::max();
		for(unsigned int iP=0; iP<polyP.size(); ++iP){
			const IBKMK::Vector2D &vP = polyP[iP];
			for(unsigned int iHole=0; iHole<tempHoles.size(); ++iHole){
				const IBKMK::Polygon2D &hole = tempHoles[iHole];
				for(unsigned int iH=0; iH<hole.vertexes().size(); ++iH){
					const IBKMK::Vector2D &vH = hole.vertexes()[iH];
					double dist =(vP-vH).magnitudeSquared();
					if(dist >= dd.m_distance)
						continue;
					dd.m_distance = dist;
					dd.m_idxPolyPoint = iP;
					dd.m_idxHole = iHole;
					dd.m_idxHolePoint = iH;
				}
			}
		}

		// add hole points to original polyline and duplicate anker point
		const std::vector<IBKMK::Vector2D> &holeVerts = tempHoles[dd.m_idxHole].vertexes();
		polyP.insert(polyP.begin() + dd.m_idxPolyPoint + 1, holeVerts.begin() + dd.m_idxHolePoint, holeVerts.end());
		polyP.insert(polyP.begin() + dd.m_idxPolyPoint + holeVerts.size() - dd.m_idxHolePoint, holeVerts.begin(), holeVerts.begin() + dd.m_idxHolePoint);
		polyP.insert(polyP.begin() + dd.m_idxPolyPoint + holeVerts.size(), polyP[dd.m_idxPolyPoint]);
		// delete hole from hole vector
		tempHoles.erase(tempHoles.begin() + dd.m_idxHole);
	}

	// check if polygon contains unnecessary snippets
	unsigned int idx=0;
	while(idx < polyP.size()+1){
		unsigned int polySize = polyP.size();
		unsigned idx0 = (idx)%polySize;
		unsigned idx1 = (idx+1)%polySize;
		unsigned idx2 = (idx+2)%polySize;
		std::vector<IBKMK::Vector2D> verts{polyP[idx0],polyP[idx1], polyP[idx2]};
		bool falsePoly = false;
		IBKMK::Polygon2D poly;
		try{
			// if colinear points inside verts then a empty polygon is returned
			poly.setVertexes(verts);
			falsePoly = poly.vertexes().empty();
		}
		catch(...){
			falsePoly = true;
		}

		// if we have an error erase the middle point and move back index by one
		if(falsePoly || poly.area(6) < MIN_AREA){
			polyP.erase(polyP.begin() + idx1);
			--idx;
			realHole = false;
		}
		else
			++idx;
		if(polyP.size()<3){
			// if there is an error, return original polygon
			newPolygon = polygon;
			return;
		}
	}

	// ist das hier valide? nein ist nicht valide
	try {
		newPolygon.setVertexes(polyP);
		if(newPolygon.vertexes().empty())
			newPolygon = polygon;
	}  catch (...) {
		newPolygon = polygon;

	}

	// Did we produce a surface with a "real" hole or did we have some numerical inconsistencies
	// all points of original polygon have to be part of the new polygon with hole, then its a
	// so called "real" hole
}

ClipperLib::Path Project::convertVec2DToClipperPath(const std::vector<IBKMK::Vector2D> &vertexes){

	ClipperLib::Path path;
	for(const IBKMK::Vector2D &p : vertexes){
		qDebug() << "Point x: " << p.m_x << " | y: " << p.m_y;
		path << ClipperLib::IntPoint(static_cast<long long>(p.m_x * SCALE_FACTOR),
										static_cast<long long>(p.m_y * SCALE_FACTOR));
	}

	return path;
}

std::vector<IBKMK::Vector2D> Project::convertClipperPathToVec2D(const ClipperLib::Path &path){
	std::vector<IBKMK::Vector2D>  poly;
	for(const ClipperLib::IntPoint &p : path)
		poly.push_back(IBKMK::Vector2D(p.X / SCALE_FACTOR, p.Y / SCALE_FACTOR));

	return poly;
}

void Project::testProjectClipping(){
	ClipperLib::Paths mainPoly;
	ClipperLib::Paths otherPoly;

	ClipperLib::Path mainPolyPath;
	ClipperLib::Path otherPolyPath, holeInside;

	// sechseck
	mainPolyPath << ClipperLib::IntPoint(0,0);
	mainPolyPath << ClipperLib::IntPoint(10,0);
	mainPolyPath << ClipperLib::IntPoint(10,10);
	mainPolyPath << ClipperLib::IntPoint(3,10);
	mainPolyPath << ClipperLib::IntPoint(3,7);
	mainPolyPath << ClipperLib::IntPoint(0,7);

	int aaa = 7;

	switch(aaa){
		case 0:{
			// dreieck ist schnittgegner
			// triangle
			otherPolyPath << ClipperLib::IntPoint(1,1);
			otherPolyPath << ClipperLib::IntPoint(2,2);
			otherPolyPath << ClipperLib::IntPoint(3,1);

			// add path to paths
			mainPoly << mainPolyPath;
			otherPoly << otherPolyPath;
		}
		break;
		case 1:{
			// viereck ist schnittgegner
			// rectangle
			otherPolyPath << ClipperLib::IntPoint(8,8);
			otherPolyPath << ClipperLib::IntPoint(9,8);
			otherPolyPath << ClipperLib::IntPoint(9,9);
			otherPolyPath << ClipperLib::IntPoint(8,9);

			// add path to paths
			mainPoly << mainPolyPath;
			otherPoly << otherPolyPath;
		}break;
		case 2:{
			// dreieck is loch vom mainPoly und das viereck der schnittgegner
			// hole triangle
			holeInside << ClipperLib::IntPoint(1,1);		// Drehrichtung beachten Prüfung?
			holeInside << ClipperLib::IntPoint(3,1);
			holeInside << ClipperLib::IntPoint(2,2);

			// rectangle
			otherPolyPath << ClipperLib::IntPoint(8,8);
			otherPolyPath << ClipperLib::IntPoint(9,8);
			otherPolyPath << ClipperLib::IntPoint(9,9);
			otherPolyPath << ClipperLib::IntPoint(8,9);

			// add path to paths
			mainPoly << mainPolyPath;
			mainPoly << holeInside;
			otherPoly << otherPolyPath;
		}break;
		case 3:{
			// dreieck ist loch vom mainPoly und das viereck der schnittgegner (dieser geht jetzt über das mainPoly hinaus und teilt es dadruch)
			// hole triangle
			holeInside << ClipperLib::IntPoint(1,1);		// Drehrichtung beachten Prüfung?
			holeInside << ClipperLib::IntPoint(3,1);
			holeInside << ClipperLib::IntPoint(2,2);

			// rectangle
			otherPolyPath << ClipperLib::IntPoint(8,-1);
			otherPolyPath << ClipperLib::IntPoint(9,-1);
			otherPolyPath << ClipperLib::IntPoint(9,11);
			otherPolyPath << ClipperLib::IntPoint(8,11);

			// add path to paths
			mainPoly << mainPolyPath;
			mainPoly << holeInside;
			otherPoly << otherPolyPath;
		}break;
		case 4:{
			// dreieck ist loch vom mainPoly (das dreieck ragt in den schnittbereich des schnittgegners hinein
			// und das viereck der schnittgegner (dieser geht jetzt über das mainPoly hinaus und teilt es dadruch)
			// hole triangle
			holeInside << ClipperLib::IntPoint(1,1);		// Drehrichtung beachten Prüfung?
			holeInside << ClipperLib::IntPoint(2,2);
			holeInside << ClipperLib::IntPoint(8,1);

			// rectangle
			otherPolyPath << ClipperLib::IntPoint(7,-1);
			otherPolyPath << ClipperLib::IntPoint(9,-1);
			otherPolyPath << ClipperLib::IntPoint(9,11);
			otherPolyPath << ClipperLib::IntPoint(7,11);

			// add path to paths
			mainPoly << mainPolyPath;
			mainPoly << holeInside;
			otherPoly << otherPolyPath;
		}break;
		case 5:{
			// dreieck und viereck sind nun mainPolys und der schnittgegner ist das sechseck
			// triangle
			holeInside << ClipperLib::IntPoint(1,1);		// Drehrichtung beachten Prüfung?
			holeInside << ClipperLib::IntPoint(2,2);
			holeInside << ClipperLib::IntPoint(3,1);

			// rectangle
			otherPolyPath << ClipperLib::IntPoint(7,1);
			otherPolyPath << ClipperLib::IntPoint(9,1);
			otherPolyPath << ClipperLib::IntPoint(9,9);
			otherPolyPath << ClipperLib::IntPoint(7,9);

			// add path to paths
			mainPoly << otherPolyPath;
			mainPoly << holeInside;
			otherPoly << mainPolyPath;
		}break;
		case 6:{
			// dreieck ist loch vom sechseck
			// fünfeck ist loch vom viereck
			// fünfeck ist schnittgegner
			// hole triangle
			holeInside << ClipperLib::IntPoint(1,1);		// Drehrichtung beachten Prüfung?
			holeInside << ClipperLib::IntPoint(2,2);
			holeInside << ClipperLib::IntPoint(3,1);

			// rectangle
			otherPolyPath << ClipperLib::IntPoint(5,1);
			otherPolyPath << ClipperLib::IntPoint(9,1);
			otherPolyPath << ClipperLib::IntPoint(9,9);
			otherPolyPath << ClipperLib::IntPoint(5,9);

			ClipperLib::Path otherPolyHolePath;
			otherPolyHolePath << ClipperLib::IntPoint(6,2);
			otherPolyHolePath << ClipperLib::IntPoint(8,2);
			otherPolyHolePath << ClipperLib::IntPoint(8,3);
			otherPolyHolePath << ClipperLib::IntPoint(7,4);
			otherPolyHolePath << ClipperLib::IntPoint(6,3);

			// add path to paths
			mainPoly << mainPolyPath;
			mainPoly << holeInside;
			otherPoly << otherPolyPath;
			otherPoly << otherPolyHolePath;
		}break;
		case 7:{
			// dreieck ist schnittgegner
			// triangle
			otherPolyPath << ClipperLib::IntPoint(1,1);
			otherPolyPath << ClipperLib::IntPoint(2,2);
			otherPolyPath << ClipperLib::IntPoint(3,1);

			ClipperLib::Path secondHole;
			secondHole << ClipperLib::IntPoint(1,2);
			secondHole << ClipperLib::IntPoint(3,5);
			secondHole << ClipperLib::IntPoint(1,5);

			// add path to paths
			mainPoly << mainPolyPath;
			mainPoly << secondHole;
			otherPoly << otherPolyPath;
		}
		break;

	}

	// cutting

	// init clipper object
	ClipperLib::Clipper clp;

	// add clipper lib paths with geometry from surfaces
	clp.AddPaths(mainPoly, ClipperLib::ptSubject, true);
	clp.AddPaths(otherPoly, ClipperLib::ptClip, true);

	ClipperLib::PolyTree polyTreeResultsIntersection;
	ClipperLib::PolyTree test1, test2;
	ClipperLib::PolyTree polyTreeResultsDiffs;

	// do finally all CLIPPINGS in CLIPPER LIB
	ClipperLib::Paths solutionIntersection, solutionDiff;
	clp.Execute(ClipperLib::ctIntersection, polyTreeResultsIntersection, ClipperLib::pftEvenOdd, ClipperLib::pftEvenOdd);
	clp.Execute(ClipperLib::ctDifference, polyTreeResultsDiffs, ClipperLib::pftEvenOdd, ClipperLib::pftEvenOdd);


	// Convert back PolyTree
	for(unsigned int i=0; i<polyTreeResultsIntersection.Childs.size(); ++i) {
	// convert all interscetion polygons
	// for(unsigned int i=0; i<solutionIntersection.size(); ++i) {
		ClipperLib::PolyNode *childNode = polyTreeResultsIntersection.Childs[i];
		const ClipperLib::Path &path = childNode->Contour;

		if(isIntersectionAnHole(path, polyTreeResultsDiffs.Childs))
			continue;

		// Add back main intersection
		//mainIntersections.push_back(extPolygon());
		//IBKMK::Polygon2D &poly = mainIntersections.back().m_polygon;

		// Convert back polygon points
		std::vector<IBKMK::Vector2D> vert2D;
		for(const ClipperLib::IntPoint &ip : path) {
			vert2D.push_back(IBKMK::Vector2D((double)ip.X/SCALE_FACTOR, (double)ip.Y/SCALE_FACTOR));
		}
		//poly.setVertexes(vert2D);

		// Should not be an hole
		Q_ASSERT(!childNode->IsHole());
	}
}


bool Project::isSamePolygon(const ClipperLib::Path &diff, const ClipperLib::Path &intersection){

	if(diff.size() != intersection.size() || diff.size()<3)
		return false;


	// find startpoint diff[0] in intersection polyline
	const ClipperLib::IntPoint &pDiff = diff[0];
	bool foundSamePoint = false;
	unsigned int idxStartDiff = 0;
	for(; idxStartDiff < intersection.size(); ++idxStartDiff){
		const ClipperLib::IntPoint &pInter = intersection[idxStartDiff];
		// check for same point
		if(pDiff == pInter){
			foundSamePoint = true;
			break;
		}
	}

	if(!foundSamePoint)
		return false;

	// check spinning direction of the two polylines
	if(diff[1] == intersection[(idxStartDiff + 1)%intersection.size()]){
		// same turning
		for(unsigned int i=2; i<diff.size(); ++i){
			const ClipperLib::IntPoint &pDiff = diff[i];
			const ClipperLib::IntPoint &pInter = intersection[(idxStartDiff + i)%intersection.size()];
			// check for same point
			if(pDiff != pInter)
				return false;
		}
	}
	else if(diff[1] == intersection[(idxStartDiff + intersection.size() -1)%intersection.size()]){
		// opposite direction
		for(unsigned int i=2; i<diff.size(); ++i){
			const ClipperLib::IntPoint &pDiff = diff[i];
			const ClipperLib::IntPoint &pInter = intersection[(idxStartDiff + intersection.size() - i)%intersection.size()];
			// check for same point
			if(pDiff != pInter)
				return false;
		}
	}
	else
		// we did not find similar points --> go out
		return false;

	return true;
}

bool Project::isIntersectionAnHole(const ClipperLib::Path &pathIntersection, const ClipperLib::PolyNodes &diffs){

	for(unsigned int i1=0; i1<diffs.size(); ++i1){
		ClipperLib::PolyNode *pn1 = diffs[i1];
		bool isPn1Hole = pn1->IsHole();
		if(isPn1Hole && isSamePolygon(pathIntersection, pn1->Contour))
			return true;
		if(isIntersectionAnHole(pathIntersection, pn1->Childs))
			return true;
	}
	return false;
}

void Project::doClipperClipping(const extPolygon &surf,
								const extPolygon &otherSurf,
								std::vector<extPolygon> &mainDiffs,
								std::vector<extPolygon> &mainIntersections,
								bool normalInterpolation) {

	ClipperLib::Paths	mainPoly(2);
	ClipperLib::Path	&polyClp = mainPoly[0];
	ClipperLib::Path	&holeClp = mainPoly.back();

	ClipperLib::PolyTree polyTreeResultsIntersection;
	ClipperLib::PolyTree polyTreeResultsDiffs;

	qDebug() << "Do clipping ...";
	qDebug() << "Surface vertices: " << surf.m_polygon.vertexes().size();

	Q_ASSERT(!surf.m_polygon.vertexes().empty());

	// set up first polygon for clipper
	qDebug() << "First Polygon for Clipping";

	// Init PolyNode
	ClipperLib::PolyNode pnMain;
	pnMain.Contour = convertVec2DToClipperPath(surf.m_polygon.vertexes());

	polyClp = convertVec2DToClipperPath(surf.m_polygon.vertexes());

	bool orientationMainPoly = ClipperLib::Orientation(polyClp);

	if(surf.m_haveRealHole) {
		// set up hole polygon
		for (unsigned int idxHole = 0; idxHole < surf.m_holePolygons.size(); ++idxHole) {
			const IBKMK::Polygon2D &holePoly = surf.m_holePolygons[idxHole];
			qDebug() << "Adding hole with Index " << idxHole << " to Clipper data structure";
			holeClp = convertVec2DToClipperPath(holePoly.vertexes());
			bool orientationHolePoly = ClipperLib::Orientation(holeClp);
			// Init PolyNode
			ClipperLib::PolyNode pnHole;
			pnHole.Contour = convertVec2DToClipperPath(holePoly.vertexes());
			pnMain.Childs.push_back(&pnHole);
		}
	}
	if(surf.m_holePolygons.empty())
		mainPoly.pop_back();


	// set up clipper lib paths
	ClipperLib::Paths	otherPoly(2);
	ClipperLib::Path	&otherPolyClp = otherPoly[0];
	ClipperLib::Path	&otherHoleClp = otherPoly.back();

	// set up second polygon for clipper
	qDebug() << "Other polygon line for clipping. ";
	otherPolyClp = convertVec2DToClipperPath(otherSurf.m_polygon.vertexes());

	// Init PolyNode
	ClipperLib::PolyNode pnOtherMain;
	pnOtherMain.Contour = otherPolyClp;

	if(otherSurf.m_haveRealHole) {
		// set up hole polygon
		for (unsigned int idxHole = 0; idxHole < otherSurf.m_holePolygons.size(); ++idxHole) {
			const IBKMK::Polygon2D &holePoly = otherSurf.m_holePolygons[idxHole];
			qDebug() << "Adding hole with Index " << idxHole << " to Clipper data structure";

			otherHoleClp = convertVec2DToClipperPath(holePoly.vertexes());

			// Init PolyNode
			ClipperLib::PolyNode pnHole;
			pnHole.Contour = convertVec2DToClipperPath(holePoly.vertexes());
			pnMain.Childs.push_back(&pnHole);
		}
	}

	if(otherSurf.m_holePolygons.empty())
		otherPoly.pop_back();

	// init clipper object
	ClipperLib::Clipper clp;


	// add clipper lib paths with geometry from surfaces
	clp.AddPaths(mainPoly, ClipperLib::ptSubject, true);
	clp.AddPaths(otherPoly, ClipperLib::ptClip, true);

	// do finally all CLIPPINGS in CLIPPER LIB
	ClipperLib::Paths solutionIntersection, solutionDiff;
	clp.Execute(ClipperLib::ctIntersection, polyTreeResultsIntersection, ClipperLib::pftEvenOdd, ClipperLib::pftEvenOdd);
	clp.Execute(ClipperLib::ctDifference, polyTreeResultsDiffs, ClipperLib::pftEvenOdd, ClipperLib::pftEvenOdd);

	/*
		In den Intersections sind dann nur Elemente die keine Löcher sind.
		Zu Prüfen ist der Fall Loch auf Loch.
	*/

	// Convert back PolyTree
	for(unsigned int i=0; i<polyTreeResultsIntersection.Childs.size(); ++i) {
	// convert all interscetion polygons
	// for(unsigned int i=0; i<solutionIntersection.size(); ++i) {
		ClipperLib::PolyNode *childNode = polyTreeResultsIntersection.Childs[i];
		const ClipperLib::Path &path = childNode->Contour;

		if(isIntersectionAnHole(path, polyTreeResultsDiffs.Childs))
			continue;
		// ToDo Dirk: Zusätzlich müssen die Intersections noch mit den Löchern geschnitten werden.

		// Add back main intersection
		mainIntersections.push_back(extPolygon());
		IBKMK::Polygon2D &poly = mainIntersections.back().m_polygon;

		// Convert back polygon points
		poly.setVertexes(convertClipperPathToVec2D(path));

		// Should not be an hole
		Q_ASSERT(!childNode->IsHole());
	}

	// Convert back PolyTree
	for(unsigned int i=0; i<polyTreeResultsDiffs.Childs.size(); ++i) {
	// convert all interscetion polygons
	// for(unsigned int i=0; i<solutionIntersection.size(); ++i) {
		ClipperLib::PolyNode *childNode = polyTreeResultsDiffs.Childs[i];
		const ClipperLib::Path &path = childNode->Contour;

		// Add back main intersection
		mainDiffs.push_back(extPolygon());
		IBKMK::Polygon2D &poly = mainDiffs.back().m_polygon;

		// Convert back points
		poly.setVertexes(convertClipperPathToVec2D(path));

		for(ClipperLib::PolyNode *secondChild : childNode->Childs){
			if(!secondChild->IsHole())
				continue;
			IBKMK::Polygon2D polyHole;
			polyHole.setVertexes(convertClipperPathToVec2D(secondChild->Contour));
			mainDiffs.back().m_holePolygons.push_back(polyHole);
		}

		// Should not be an hole
		Q_ASSERT(!childNode->IsHole());
	}
}
}



const VICUS::Project &RC::Project::newPrjVicus() const {
	return m_newPrjVicus;
}

