/*	The RoomClipper data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Stephan Hirth     <stephan.hirth -[at]- tu-dresden.de>
	  Dirk Weiß         <dirk.weis     -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/


#include <IBKMK_3DCalculations.h>
#include <VICUS_Object.h>

#include <IBK_StopWatch.h>

#include "RC_VicusClipping.h"
#include "RC_ClippingSurface.h"

#if defined(_OPENMP)
#include <omp.h> // needed for omp_get_num_threads()
#endif

namespace RC {

void VicusClipper::addClipperPolygons(const std::vector<ClippingPolygon> &polysTemp, std::vector<ClippingPolygon> &polys) {
	for(const ClippingPolygon &polyTemp : polysTemp) {
		bool foundPolygon = false;
		for(const ClippingPolygon &poly : polys) {
			if(polyTemp == poly) {
				foundPolygon = true;
				break;
			}
		}
		if(!foundPolygon) {
			if(!polyTemp.m_polygon.isValid()) {
				continue;
			}
			if(polyTemp.m_polygon.area() > MIN_AREA)
				polys.push_back(polyTemp);
		}
	}
}


void insertChildSurfaces(std::set<const VICUS::Surface*> &surfaces, const VICUS::Surface &s, bool onlySelected) {
	for(const VICUS::Surface &cs : s.childSurfaces()) {

		bool selected = true;
		if(onlySelected && !s.m_selected)
			selected = false;

		if(selected)
			surfaces.insert(&cs);

		insertChildSurfaces(surfaces, cs, onlySelected);
	}
}


void VicusClipper::findParallelSurfaces(Notification *notify) {
	FUNCID(VicusClipper::findParallelSurfaces);

	IBK::StopWatch totalTimer;
	totalTimer.start();
	// the stop watch object and progress counter are used only in a critical section
	IBK::StopWatch w;
	w.start();
	notify->notify(0);

	std::set<const VICUS::Surface*>	surfaces;

	for(const VICUS::Building &b : m_vicusBuildings) {
		for(const VICUS::BuildingLevel &bl : b.m_buildingLevels) {
			for(const VICUS::Room &r : bl.m_rooms) {
				for(const VICUS::Surface &s : r.m_surfaces) {
					if(m_onlySelected && !s.m_selected)
						continue;
					surfaces.insert(const_cast<VICUS::Surface*>(&s));
				}
			}
		}
	}

	std::set<unsigned int> alreadyCoupledSurfaces;
	for(const VICUS::ComponentInstance &ci : m_vicusCompInstances) {
		if(ci.m_idSideASurface != VICUS::INVALID_ID && ci.m_idSideBSurface != VICUS::INVALID_ID) {
			alreadyCoupledSurfaces.insert(ci.m_idSideASurface);
			alreadyCoupledSurfaces.insert(ci.m_idSideBSurface);
		}
	}

	unsigned int Count = surfaces.size() * surfaces.size();
	unsigned int currentCount = 0;

	for(const VICUS::Surface *s1 : surfaces){

		// Skip already coupled surfaces
		if(alreadyCoupledSurfaces.find(s1->m_id) != alreadyCoupledSurfaces.end())
			continue;

		for(const VICUS::Surface *s2 : surfaces){

			++currentCount;
			// only notify every second or so
			if (!notify->m_aborted && w.difference() > 100) {
				notify->notify(0.25 * double(currentCount+1) / Count);
				w.start();
			}

			if (notify->m_aborted)
				throw IBK::Exception("Clipping canceled.", FUNC_ID);

			// Skip already coupled surfaces
			if(alreadyCoupledSurfaces.find(s2->m_id) != alreadyCoupledSurfaces.end())
				continue;

			// skip same surfaces
			if(s1 == s2)
				continue;

			const VICUS::Surface &surf1 = *s1;
			const VICUS::Surface &surf2 = *s2;

			if(surf1.m_parent->m_id == surf2.m_parent->m_id)
				continue; // only surfaces of different rooms are clipped

			// skip already handled surfaces
			if(m_surfaceConnections.find(surf1.m_id) != m_surfaceConnections.end())
				if(m_surfaceConnections[surf1.m_id].find(surf2.m_id) != m_surfaceConnections[surf1.m_id].end())
					continue;

			// calculation of normal deviation
			double angle = IBKMK::angleBetweenVectorsDeg(-1 * surf1.geometry().normal(), surf2.geometry().normal());

			// check if deviation is inside limits
			if(angle > m_normalDeviationInDeg)
				continue;

			// save parallel surfaces
			ClippingSurface &cs = findClippingSurface(surf1.m_id, m_vicusBuildings);
			cs.m_clippingObjects.push_back(ClippingObject(surf2.m_id, surf2, 999) );

			//
			ClippingSurface &cs2 = findClippingSurface(surf2.m_id, m_vicusBuildings);
			cs2.m_clippingObjects.push_back(ClippingObject(surf1.m_id, surf1, 999) );

			// qDebug() << s1->m_id << ": " << s1->m_displayName << " | " << s2->m_id << ": "<< s2->m_displayName ;

			m_surfaceConnections[surf1.m_id].insert(surf2.m_id);
			m_surfaceConnections[surf2.m_id].insert(surf1.m_id);


		}

		IBK::IBK_Message(IBK::FormatString("Found connections for '%1 | %2'")
						 .arg(s1->m_parent->m_displayName.toStdString())
						 .arg(s1->m_displayName.toStdString()), IBK::MSG_PROGRESS);
	}
}


void VicusClipper::findSurfacesInRange(Notification *notify) {
	FUNCID(VicusClipper::findSurfacesInRange);

	IBK::StopWatch totalTimer;
	totalTimer.start();
	// the stop watch object and progress counter are used only in a critical section
	IBK::StopWatch w;
	w.start();

	unsigned int Count = m_surfaceConnections.size();
	unsigned int currentCount = 0;

	for(std::map<unsigned int, std::set<unsigned int>>::iterator it = m_surfaceConnections.begin();
		it != m_surfaceConnections.end(); ++it){

		++currentCount;
		// only notify every second or so
		if (!notify->m_aborted && w.difference() > 100) {
			notify->notify(0.25 + 0.25 * double(currentCount+1) / Count);
			w.start();
		}


		if (notify->m_aborted)
			throw IBK::Exception("Clipping canceled.", FUNC_ID);

		// look for clipping surface
		ClippingSurface &cs = findClippingSurface(it->first, m_vicusBuildings);
		const VICUS::Surface &s1 = cs.m_vicusSurface;

		unsigned int surfCounter = 0;

		std::vector<ClippingObject> newClippingObjects;
		for(unsigned int id2 : it->second){

			const VICUS::Surface &surf2 = findVicusSurface(id2, m_vicusBuildings);

			// get our co object
			unsigned int idx2 = 0 ;
			for(;idx2<cs.m_clippingObjects.size(); ++idx2) {
				if(surf2.m_id == cs.m_clippingObjects[idx2].m_vicusId)
					break;
			}
			ClippingObject &co = cs.m_clippingObjects[idx2];
			const VICUS::Surface &s2 = co.m_vicusSurface;

			// clipping object is for normalized directional vectors equal the distance of the two points
			IBKMK::Vector3D rayEndPoint;
			IBKMK::lineToPointDistance( s1.geometry().offset(), s1.geometry().normal().normalized(),
										s2.geometry().offset(), co.m_distance, rayEndPoint);
			if(co.m_distance > m_maxDistanceOfSurfaces + EPSILON || co.m_distance < 0)
				continue;

			newClippingObjects.push_back(co);

			++surfCounter;
		}
		std::sort(newClippingObjects.begin(), newClippingObjects.end());
		// swap old clipping objects with newly sorted and in range surfaces
		cs.m_clippingObjects.swap(newClippingObjects);

		IBK::IBK_Message(IBK::FormatString("Found %1 surfaces in range of surface '%2 | %3'")
						 .arg(surfCounter, 4)
						 .arg(s1.m_displayName.toStdString())
						 .arg(s1.m_parent->m_displayName.toStdString()), IBK::MSG_PROGRESS);
	}
}


void VicusClipper::addSurfaceToClippingPolygons(const VICUS::Surface &surf, std::vector<ClippingPolygon> &clippingPolygons) {
	if(surf.childSurfaces().empty())
		clippingPolygons.push_back(surf.geometry().polygon2D());
	else {
		std::vector<IBKMK::Polygon2D> holes;
		for(const VICUS::PlaneGeometry::Hole &h : surf.geometry().holes()) {
			if(h.m_isChildSurface)
				holes.push_back(h.m_holeGeometry);
		}
		clippingPolygons.push_back(ClippingPolygon(surf.geometry().polygon2D(), holes));
	}
}

const std::vector<VICUS::ComponentInstance>* VicusClipper::vicusCompInstances() const {
	return &m_vicusCompInstances;
}

const std::vector<VICUS::Building> VicusClipper::vicusBuildings() const {
	return m_vicusBuildings;
}


void VicusClipper::clipSurfaces(Notification * notify) {

	FUNCID(VicusClipper::clipSurfaces);

	IBK::StopWatch totalTimer;
	totalTimer.start();
	// the stop watch object and progress counter are used only in a critical section
	IBK::StopWatch w;
	w.start();

	//#if defined(_OPENMP)
	//    int threadCount = 1;

	//#pragma omp parallel
	//    {
	//        if (omp_get_thread_num() == 0) {
	//            threadCount = omp_get_num_threads();
	//            // we should leave one CPU free for the GUI update
	//            if (threadCount > 4) {
	//                --threadCount;
	//                omp_set_num_threads(threadCount);
	//            }
	//            IBK::IBK_Message(IBK::FormatString("Running clipping calculation in parallel with %1 threads.\n").arg(threadCount));
	//        }
	//    }
	//#endif

	unsigned int connectionCount = m_surfaceConnections.size();
	unsigned int currentConnectionCount = 0;

	//#if defined(_OPENMP)
	//#pragma omp parallel for schedule(dynamic, 1)
	//#endif
	for(std::map<unsigned int, std::set<unsigned int>>::const_iterator it = m_surfaceConnections.begin();
		it != m_surfaceConnections.end(); ++it){

		// master thread 0 updates the progress dialog; this should be good enough for longer runs
		//#if defined(_OPENMP)
		//        if ( omp_get_thread_num() == 0) {
		//#endif
		// only notify every second or so
		if (!notify->m_aborted && w.difference() > 100) {
			notify->notify(0.5 + 0.25*double(currentConnectionCount+1) / connectionCount);
			w.start();
		}

		if (notify->m_aborted)
			throw IBK::Exception("Clipping canceled.", FUNC_ID);
		//#if defined(_OPENMP)
		//            }
		//#endif

		//#if defined(_OPENMP)
		//#pragma omp critical
		//#endif
		++currentConnectionCount;

		// look for clipping surface
		//#if defined(_OPENMP)
		//#pragma omp critical
		//#endif

		ClippingSurface &cs = findClippingSurface(it->first, m_vicusBuildings);

		// original surface & 1 Copy
		VICUS::Surface originSurf = cs.m_vicusSurface;
		VICUS::Surface originSurfCopy = originSurf;

		// Store original id of surface
		unsigned int surfOriginId = originSurf.m_id;

		// Hold data of orifinal surface
		const IBKMK::Vector3D &localX = originSurf.geometry().localX();
		const IBKMK::Vector3D &localY = originSurf.geometry().localY();
		const IBKMK::Vector3D &offset = originSurf.geometry().offset();

		// Hold display name
		QString displayName = originSurf.m_displayName;

		// Pointer to current room
		VICUS::Room *r = dynamic_cast<VICUS::Room*>(originSurf.m_parent);

		if(r == nullptr) {
			continue;
		}

		// init all cutting objects
		std::vector<ClippingPolygon> mainDiffs, mainIntersections, clippingPolygons;
		addSurfaceToClippingPolygons(originSurf, clippingPolygons);

		// we need the connection to the construction instance, save it!
		if(cs.m_clippingObjects.empty()){
			if(originSurf.m_componentInstance == nullptr)
				continue;
			m_compInstOriginSurfId[originSurfCopy.m_id] = originSurf.m_componentInstance->m_id;
			continue;
		}

		// delete original surfaces
		unsigned int eraseIdx = 0;
		for(;eraseIdx<r->m_surfaces.size(); ++eraseIdx){
			if(r->m_surfaces[eraseIdx].m_id == originSurf.m_id)
				break;
		}

		// Erase origin surface
		//#if defined(_OPENMP)
		//#pragma omp critical
		//#endif
		r->m_surfaces.erase(r->m_surfaces.begin()+eraseIdx);
		//#if defined(_OPENMP)
		//#pragma omp critical
		//#endif
		r->updateParents();

		// Store room surface count
		unsigned int roomSurfaceCount = r->m_surfaces.size();

		// Temporarily store all original sub-surfaces
		std::vector<VICUS::SubSurface> originalSubSurfaces = originSurf.subSurfaces();

		// Iterate through all found possible clipping objects
		for(ClippingObject &co : cs.m_clippingObjects){
			const VICUS::Surface &s2 = co.m_vicusSurface;
			IBKMK::Polygon2D hole;

			// calculate new projection points onto our main polygon plane (clipper works 2D)
			std::vector<IBKMK::Vector2D> vertexes(s2.geometry().polygon2D().vertexes().size());

			for(unsigned int i=0; i<vertexes.size(); ++i){

				// If we have no surface vertexes, we skip it
				if(s2.geometry().polygon3D().vertexes().empty())
					continue;

				const IBKMK::Vector3D &p = s2.geometry().polygon3D().vertexes()[i];
				IBKMK::Vector3D pNew = p-co.m_distance*originSurf.geometry().normal();

				try {
					IBKMK::planeCoordinates(offset, localX, localY,
											pNew, vertexes[i].m_x, vertexes[i].m_y);

				}  catch (...) {
					continue;
				}
			}

			std::vector<ClippingPolygon> mainDiffsTemp, mainIntersectionsTemp;
			unsigned int maxSize = clippingPolygons.size();
			for(unsigned int i=0; i<maxSize; ++i){
				// do clipping with clipper lib
				doClipperClipping(clippingPolygons.back(), ClippingPolygon(vertexes), mainDiffsTemp, mainIntersectionsTemp);
				clippingPolygons.pop_back();

				addClipperPolygons(mainDiffsTemp, mainDiffs);
				addClipperPolygons(mainIntersectionsTemp, mainIntersections);
			}

			// main intersection saving
			for(ClippingPolygon &poly : mainIntersections) {

				if(!poly.m_polygon.isValid())
					continue;

				IBK::IBK_Message(IBK::FormatString("Surface '%1 | %2' is beeing clipped by surface '%3 | %4'")
								 .arg(originSurf.m_parent->m_displayName.toStdString())
								 .arg(originSurf.m_displayName.toStdString())
								 .arg(s2.m_parent->m_displayName.toStdString())
								 .arg(s2.m_displayName.toStdString()), IBK::MSG_PROGRESS);

				try {
					originSurf.m_id = ++m_nextVicusId;
					originSurf.m_displayName = generateUniqueName(displayName);

					// calculate new offset 3D
					IBKMK::Vector3D newOffset3D = offset	+ localX * poly.m_polygon.vertexes()[0].m_x
							+ localY * poly.m_polygon.vertexes()[0].m_y;

					// calculate new ofsset 2D
					IBKMK::Vector2D newOffset2D = poly.m_polygon.vertexes()[0];

					// move our points
					for(const IBKMK::Vector2D &v : poly.m_polygon.vertexes())
						const_cast<IBKMK::Vector2D &>(v) -= newOffset2D;

					// update VICUS Surface with new geometry
					const_cast<IBKMK::Polygon2D&>(originSurf.geometry().polygon2D()).setVertexes(poly.m_polygon.vertexes());
					IBKMK::Polygon3D poly3D = originSurf.geometry().polygon3D();
					poly3D.setTranslation(newOffset3D);
					originSurf.setPolygon3D(poly3D);		// now marked dirty = true

					// Remove original window
					originSurf.setChildAndSubSurfaces(std::vector<VICUS::SubSurface>(), originSurf.childSurfaces());


				}
				catch (IBK::Exception &ex) {
					IBK::IBK_Message(IBK::FormatString("Surface '%1 | %2' is broken after clipping, using the original surface geometry.")
									 .arg(originSurf.m_parent->m_displayName.toStdString())
									 .arg(originSurf.m_displayName.toStdString()), IBK::MSG_ERROR);
					originSurf = originSurfCopy;
				}

				//#if defined(_OPENMP)
				//#pragma omp critical {
				//#endif
				r->m_surfaces.push_back(originSurf);

				// save id origin
				if(surfOriginId != VICUS::INVALID_ID && originSurf.m_componentInstance != nullptr)
					m_compInstOriginSurfId[originSurf.m_id] = originSurf.m_componentInstance->m_id;

				r->updateParents();
				//#if defined(_OPENMP)
				//}
				//#endif
			}

			// check diff for valid ...

			std::vector<unsigned int>	erasePos;

			for(unsigned int idx = 0; idx<mainDiffs.size(); ++idx){
				ClippingPolygon &diffPoly = mainDiffs[idx];
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

		/// All polygons that could not be cutted are remaining as rests.
		/// So if we cut polygons with windows, we have to project them back
		/// And we also have to check if the windows remain inside the surfaces.
		for(ClippingPolygon &poly : clippingPolygons) {

			if(!poly.m_polygon.isValid())
				continue;
			//#if defined(_OPENMP)
			//#pragma omp critical {
			//#endif
			// now we have an polygon, which is identical to the new clipping polygon -> so we take the old one
			if(clippingPolygons.size() == 1 && poly.m_holePolygons.empty() && r->m_surfaces.size() == roomSurfaceCount){
				r->m_surfaces.push_back(originSurfCopy);
				r->updateParents();
				if(originSurf.m_componentInstance != nullptr)
					m_compInstOriginSurfId[originSurfCopy.m_id] = originSurf.m_componentInstance->m_id;
				continue;
			}
			//#if defined(_OPENMP)
			//}
			//#endif

			originSurf.m_id = ++m_nextVicusId;

			// Only add [1] if we do have more then 1 clipping polygons
			originSurf.m_displayName = generateUniqueName(displayName);

			// calculate new offset 3D
			IBKMK::Vector3D newOffset3D = offset	+ localX * poly.m_polygon.vertexes()[0].m_x
					+ localY * poly.m_polygon.vertexes()[0].m_y;
			// calculate new ofsset 2D
			IBKMK::Vector2D newOffset2D = poly.m_polygon.vertexes()[0];

			// move our points
			for(const IBKMK::Vector2D &v : poly.m_polygon.vertexes())
				const_cast<IBKMK::Vector2D &>(v) -= newOffset2D;

			// update VICUS Surface with new geometry
			const_cast<IBKMK::Polygon2D&>(originSurf.geometry().polygon2D()).setVertexes(poly.m_polygon.vertexes());
			IBKMK::Polygon3D poly3D = originSurf.geometry().polygon3D();
			poly3D.setTranslation(newOffset3D);
			originSurf.setPolygon3D(poly3D);		// now marked dirty = true

			// Reset all child and sub-surfaces
			originSurf.setChildAndSubSurfaces(originSurfCopy.subSurfaces(), std::vector<VICUS::Surface>());

			// ==========================
			// CRAZY HOLE ACTION INCOMING
			// Now we start to handle all holes

			std::vector<VICUS::Surface> childSurfaces = originSurf.childSurfaces();
			if(poly.m_haveRealHole && poly.m_holePolygons.size() > 0) {

				std::vector<VICUS::Polygon2D> holes(poly.m_holePolygons.size());

				for(unsigned int i=0; i<poly.m_holePolygons.size(); ++i) {
					IBKMK::Polygon2D &holePoly = poly.m_holePolygons[i];
					std::vector<IBKMK::Vector3D> vertexes(holePoly.vertexes().size());

					std::vector<IBKMK::Vector2D> holePoints(holePoly.vertexes().size());
					for(unsigned int j=0; j<holePoly.vertexes().size(); ++j) {
						const IBKMK::Vector2D &v2d = holePoly.vertexes()[j];

						vertexes[j] = offset + localX * v2d.m_x
								+ localY * v2d.m_y;

						IBKMK::planeCoordinates(newOffset3D, originSurf.geometry().localX(),
												originSurf.geometry().localY(), vertexes[j], holePoints[j].m_x, holePoints[j].m_y);
					}

					VICUS::Surface childSurf = originSurf;
					childSurf.setPolygon3D(vertexes);
					childSurf.m_id = ++m_nextVicusId;
					childSurf.m_displayName = QString("%1 - Child Surface [%2]").arg(originSurf.m_displayName ).arg(i);

					IBKMK::Vector3D normalDiff = childSurf.geometry().normal() + originSurf.geometry().normal();
					if(normalDiff.magnitudeSquared() < 1)
						childSurf.flip();

					childSurfaces.push_back(childSurf);
				}
			}


			/// ==========================
			/// Update sub-surfaces ======
			/// If we have outside surface with windows and partially covered surfaces, that have been connected by the clipper
			/// We have to move the windows to the difference (rest) surfaces. Since connecting surfaces are not allowed right now
			/// to contain windows.

			if(!originSurfCopy.subSurfaces().empty()) {

				// Cache original surface data
				const IBKMK::Vector3D &originLocalX = originSurfCopy.geometry().localX();
				const IBKMK::Vector3D &originLocalY = originSurfCopy.geometry().localY();
				const IBKMK::Vector3D &originOffset = originSurfCopy.geometry().offset();

				// Update geometry
				originSurf.geometry().isValid();

				// For better usage
				const IBKMK::Vector3D &localX = originSurf.geometry().localX();
				const IBKMK::Vector3D &localY = originSurf.geometry().localY();
				const IBKMK::Vector3D &offset = originSurf.geometry().offset();

				// We copy our sub-surfaces
				std::vector<VICUS::SubSurface> subs = originSurfCopy.subSurfaces();

				// Now we should update all sub-surfaces
				for(unsigned int idxSub=0; idxSub<originSurfCopy.subSurfaces().size(); ++idxSub) {
					VICUS::SubSurface &sub = subs[idxSub];

					std::vector<IBKMK::Vector2D> points(sub.m_polygon2D.vertexes().size());

					for(unsigned int i=0; i<sub.m_polygon2D.vertexes().size(); ++i) {

						IBKMK::Vector3D v3D = originOffset + originLocalX * sub.m_polygon2D.vertexes()[i].m_x
								+ originLocalY * sub.m_polygon2D.vertexes()[i].m_y;

						IBKMK::planeCoordinates(offset, localX, localY, v3D, points[i].m_x, points[i].m_y);
					}

					sub.m_polygon2D = points;
				}

				// Update all child and sub-surfaces
				originSurf.setChildAndSubSurfaces(subs, childSurfaces);
			}

			//#if defined(_OPENMP)
			//#pragma omp critical
			//#endif
			originSurf.updateParents();

			// Add back holes to data structure
			//#if defined(_OPENMP)
			//#pragma omp critical
			//#endif
			r->m_surfaces.push_back(originSurf);

			// save id origin
			if(surfOriginId != VICUS::INVALID_ID && originSurf.m_componentInstance != nullptr)
				m_compInstOriginSurfId[originSurf.m_id] = originSurf.m_componentInstance->m_id;
			//#if defined(_OPENMP)
			//#pragma omp critical
			//#endif
			r->updateParents();
		}
	}
}


unsigned int VicusClipper::findComponentInstanceForSurface(unsigned int id){
	for(unsigned int i=0; i<m_vicusCompInstances.size(); ++i){
		VICUS::ComponentInstance& ci = m_vicusCompInstances[i];
		if(ci.m_id == id){
			return ci.m_idComponent;
		}
	}

	return VICUS::INVALID_ID;
}

QString VicusClipper::generateUniqueName(QString name) {
	int idx1 = name.lastIndexOf("[");
	int idx2 = name.lastIndexOf("]");

	if(idx1 != -1 && idx2 != -1) {
		QString strIdx = name.mid(idx1+1, idx2-idx1-1);
		bool ok;
		strIdx.toUInt(&ok);
		if(!ok)
			idx1 = -1;
	}

	QString baseName = name.left(idx1-1);

	if(m_nameMap.find(baseName) != m_nameMap.end())
		m_nameMap[baseName]++;
	else {
		m_nameMap[baseName] = 1;
		return QString("%1").arg(baseName);
	}


	return QString("%1 (%2)").arg(baseName).arg(m_nameMap[baseName]);
}


void VicusClipper::createComponentInstances(Notification *notify, bool createConnections) {
	FUNCID(VicusClipper::createComponentInstances);

	std::set<const VICUS::Surface *>	surfaces;

	// vector of new construction instances
	std::vector<VICUS::ComponentInstance>	cis;

	// list all surfaces in a set
	for(const VICUS::Building &b : m_vicusBuildings){
		for(const VICUS::BuildingLevel &bl : b.m_buildingLevels){
			for(const VICUS::Room &r : bl.m_rooms){
				for(const VICUS::Surface &s : r.m_surfaces){
					bool selected = true;
					if(m_onlySelected && !s.m_selected)
						selected = false;

					if(selected)
						surfaces.insert(&s);
					insertChildSurfaces(surfaces, s, m_onlySelected);
				}
			}
		}
	}

	if(m_onlySelected) {
		for(unsigned int i=0; i<m_vicusCompInstances.size(); ++i) {
			VICUS::ComponentInstance &ci = m_vicusCompInstances[i];
			bool foundSurf = false;
			for(const VICUS::Surface *s : surfaces) {
				// We need to store untouched cis
				if(ci.m_idSideASurface == s->m_id || ci.m_idSideBSurface == s->m_id) {
					foundSurf = true;
					break;
				}
			}
			if(!foundSurf)
				cis.push_back(ci);
		}
	}

	unsigned int &nextUnusedId = ++m_nextVicusId;

	// set for already handled surfaces
	std::set<unsigned int>					handledSurfaces;

	IBK::StopWatch totalTimer;
	totalTimer.start();
	// the stop watch object and progress counter are used only in a critical section
	IBK::StopWatch w;
	w.start();
	//notify->notify(0);

	unsigned int Count = surfaces.size();
	unsigned int currentCount = 0;

	for(const VICUS::Surface * surfA : surfaces){

		++currentCount;
		// only notify every second or so
		if (!notify->m_aborted && w.difference() > 100) {
			notify->notify(0.75 + 0.25 * double(currentCount+1) / Count);
			w.start();
		}

		if (notify->m_aborted)
			throw IBK::Exception("Clipping canceled.", FUNC_ID);

		if(handledSurfaces.find(surfA->m_id) != handledSurfaces.end())
			continue;

		// get old construction instance properties -> component id
		// qDebug() << "Surface " << surfA->m_displayName << " is cut.";
		bool foundComponentInstance = false;
		if(m_compInstOriginSurfId.find(surfA->m_id) == m_compInstOriginSurfId.end()){
			for(const VICUS::ComponentInstance &ciObj : m_vicusCompInstances){
				if(ciObj.m_idSideASurface == surfA->m_id || ciObj.m_idSideBSurface == surfA->m_id){

					unsigned int idA = ciObj.m_idSideASurface;
					unsigned int idB = ciObj.m_idSideBSurface;

					if(idA != VICUS::INVALID_ID) {
						if(m_compInstOriginSurfId.find(idA) != m_compInstOriginSurfId.end())
							handledSurfaces.insert(m_compInstOriginSurfId[idA]);
						else
							handledSurfaces.insert(idA);
					}

					if (idB != VICUS::INVALID_ID) {
						if(m_compInstOriginSurfId.find(idB) != m_compInstOriginSurfId.end())
							handledSurfaces.insert(m_compInstOriginSurfId[idB]);
						else
							handledSurfaces.insert(idB);
					}
					const_cast<VICUS::ComponentInstance &>(ciObj).m_id = nextUnusedId++; // update Id
					cis.push_back(ciObj);
					foundComponentInstance = true;
					break;
				}
			}
			// continue;
		}

		if(foundComponentInstance)
			continue;

		unsigned int compId = findComponentInstanceForSurface(m_compInstOriginSurfId[surfA->m_id]);
		VICUS::ComponentInstance ci(nextUnusedId++, compId, surfA->m_id, VICUS::INVALID_ID);

		// We only couple surfaces when we want to create new connections
		if(createConnections)
			for(const VICUS::Surface * surfB : surfaces){

				// do some checks so that we have no nullptr, already handled, adiabatic surfaces, etc...
				if(surfA == surfB || surfA == nullptr || surfB == nullptr)
					continue;
				if(surfA->m_parent == nullptr || surfB->m_parent == nullptr)
					continue;
				if(surfA->m_parent == surfB->m_parent)
					continue;

				if(handledSurfaces.find(surfA->m_id) != handledSurfaces.end() ||
						handledSurfaces.find(surfB->m_id) != handledSurfaces.end())
					continue;

				double surfaceRatio = 0;
				try {
					double areaA = surfA->geometry().area(2);
					double areaB = surfB->geometry().area(2);

					surfaceRatio = std::abs(1 - areaA/areaB);
				}
				catch(IBK::Exception &ex) {
					IBK::IBK_Message(IBK::FormatString("Polygon '%1' broken. Trying to heal it.")
									 .arg(surfA->m_displayName.toStdString()), IBK::MSG_ERROR, FUNC_ID);

					const_cast<VICUS::Polygon3D&>(surfA->geometry().polygon3D()).setVertexes(surfA->geometry().polygon3D().rawVertexes());
					const_cast<VICUS::Polygon3D&>(surfB->geometry().polygon3D()).setVertexes(surfB->geometry().polygon3D().rawVertexes());

					if(!surfA->geometry().isValid() && !surfB->geometry().isValid())
						continue;

					double areaA = surfA->geometry().area(2);
					double areaB = surfB->geometry().area(2);

					surfaceRatio = std::abs(1 - areaA/areaB);
				}

				// check area of both surfaces
				if(surfaceRatio > 0.05)
					continue;

				//qDebug() << "Fläche " << surfA->m_displayName << " wird mit Fläche " << surfB->m_displayName<< " geschnitten.";

				// convert 3D points of surface B into 2D plane of surface A
				VICUS::Surface * s1 = const_cast<VICUS::Surface*>(surfA);
				VICUS::Surface * s2 = const_cast<VICUS::Surface*>(surfB);

				const IBKMK::Vector3D &localX = s1->geometry().localX();
				const IBKMK::Vector3D &localY = s1->geometry().localY();
				const IBKMK::Vector3D &offset = s1->geometry().offset();
				double distance = 0;
				// calculate distance of these two surfaces
				// clipping object is for normalized directional vectors equal the distance of the two points
				IBKMK::Vector3D rayEndPoint;
				IBKMK::lineToPointDistance( s1->geometry().offset(), s1->geometry().normal().normalized(),
											s2->geometry().offset(), distance, rayEndPoint);
				if(distance > m_maxDistanceOfSurfaces + EPSILON || distance < 0)
					continue;

				std::vector<IBKMK::Vector2D> vertexes(s2->geometry().polygon2D().vertexes().size());
				for(unsigned int i=0; i<vertexes.size(); ++i){

					if(s2->geometry().polygon3D().vertexes().empty())
						continue;

					const IBKMK::Vector3D &p = s2->geometry().polygon3D().vertexes()[i];
					// qDebug() << "Point outside plane x: " << p.m_x << "y: " << p.m_y << "z: " << p.m_z;
					IBKMK::Vector3D pNew = p-distance*s1->geometry().normal();
					// qDebug() << "Point inside plane x: " << pNew.m_x << "y: " << pNew.m_y << "z: " << pNew.m_z;
					// project points onto the plane
					try {
						IBKMK::planeCoordinates(offset, localX, localY,
												pNew, vertexes[i].m_x, vertexes[i].m_y);

					}  catch (...) {
						continue;
					}
				}

				std::vector<ClippingPolygon> diffs, intersections;

				doClipperClipping(ClippingPolygon(s1->geometry().polygon2D().vertexes()),
								  ClippingPolygon(vertexes), diffs, intersections);


				if(intersections.size() == 1 &&
						intersections[0].m_polygon.isValid() &&
						IBK::nearly_equal<1>(intersections[0].m_polygon.area(), surfA->geometry().area())){

					unsigned int compIdB = findComponentInstanceForSurface(m_compInstOriginSurfId[s2->m_id]);
					// find old components
					if(compId == VICUS::INVALID_ID)
						compId = compIdB;

					// if both ids are invalid take invalid
					// qDebug() << "Fläche " << surfA->m_displayName << " wird mit Fläche " << surfB->m_displayName<< " gekoppelt.";

					IBK::FormatString roomString = IBK::FormatString("'%1 | %2'")
														 .arg(s1->m_displayName.toStdString())
														 .arg(s1->m_parent->m_displayName.toStdString());

					QString text = QString("%1").arg(QString::fromStdString(roomString.str()), 100);

					IBK::IBK_Message(IBK::FormatString("%1 %2 <-> %3 %4")
									 .arg(s1->m_parent->m_displayName.toStdString(), 20, std::ios_base::left)
									 .arg(s1->m_displayName.toStdString(), 20, std::ios_base::left)
									 .arg(s2->m_parent->m_displayName.toStdString(), 20, std::ios_base::left)
									 .arg(s2->m_displayName.toStdString(), 20, std::ios_base::left), IBK::MSG_PROGRESS);

					// build new component
					ci = VICUS::ComponentInstance(nextUnusedId++, compId, s1->m_id, s2->m_id);
					//handledSurfaces.insert(s1->m_id);
					handledSurfaces.insert(s2->m_id);
					break;
				}
			}
		if(handledSurfaces.find(surfA->m_id) == handledSurfaces.end()) {
			cis.push_back(ci);
			handledSurfaces.insert(surfA->m_id);
		}
	}

	// Now stirp all completly broken cis
	// Remove old component instance connection
	std::vector<unsigned int> idxs;
	for(unsigned int i=0; i<cis.size(); ++i) {
		VICUS::ComponentInstance &ci = cis[i];
		if(ci.m_idSideASurface == VICUS::INVALID_ID && ci.m_idSideBSurface == VICUS::INVALID_ID)
			idxs.push_back(i);
	}
	for(unsigned int idx=idxs.size(); idx>0; --idx){
		cis.erase(cis.begin()+idxs[idx]);
	}

	m_vicusCompInstances.swap(cis);

	notify->notify(1);
}


ClippingSurface & VicusClipper::findClippingSurface(unsigned int id, const std::vector<VICUS::Building> &buildings) {
	unsigned int idx = 0 ;
	bool found = false;
	for(;idx<m_clippingSurfaces.size(); ++idx) {
		if(id == m_clippingSurfaces[idx].m_vicusId) {
			found = true;
			break;
		}
	}
	if (!found) {
		const VICUS::Surface &s = findVicusSurface(id, buildings);
		m_clippingSurfaces.push_back(ClippingSurface(id, s));
		return m_clippingSurfaces.back();
	}

	return m_clippingSurfaces[idx];
}

const VICUS::Surface &VicusClipper::findVicusSurface(unsigned int id, const std::vector<VICUS::Building> &buildings) {
	const VICUS::Surface *surf = nullptr;
	for(const VICUS::Building & b : buildings) {
		for(const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			for(const VICUS::Room & r : bl.m_rooms) {
				for(const VICUS::Surface & s : r.m_surfaces) {
					if(s.m_id == id) {
						surf = &s;
						break;
					}
				}
			}
		}
	}
	Q_ASSERT(surf != nullptr);
	return *surf;
}


ClipperLib::Path VicusClipper::convertVec2DToClipperPath(const std::vector<IBKMK::Vector2D> &vertexes){

	ClipperLib::Path path;
	for(const IBKMK::Vector2D &p : vertexes){
		// qDebug() << "Point x: " << p.m_x << " | y: " << p.m_y;
		path << ClipperLib::IntPoint(static_cast<long long>(p.m_x * SCALE_FACTOR),
									 static_cast<long long>(p.m_y * SCALE_FACTOR));
	}

	return path;
}


std::vector<IBKMK::Vector2D> VicusClipper::convertClipperPathToVec2D(const ClipperLib::Path &path){
	std::vector<IBKMK::Vector2D>  poly;
	for(const ClipperLib::IntPoint &p : path)
		poly.push_back(IBKMK::Vector2D((double)p.X / SCALE_FACTOR, (double)p.Y / SCALE_FACTOR));

	return poly;
}


bool VicusClipper::isSamePolygon(const ClipperLib::Path &diff, const ClipperLib::Path &intersection){

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


bool VicusClipper::isIntersectionAnHole(const ClipperLib::Path &pathIntersection, const ClipperLib::PolyNodes &diffs){

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


void VicusClipper::doClipperClipping(const ClippingPolygon &surf,
									 const ClippingPolygon &otherSurf,
									 std::vector<ClippingPolygon> &mainDiffs,
									 std::vector<ClippingPolygon> &mainIntersections,
									 bool normalInterpolation) {

	ClipperLib::Paths	mainPoly(1+surf.m_holePolygons.size());
	ClipperLib::Path	&polyClp = mainPoly[0];
	//ClipperLib::Path	&holeClp = mainPoly.back();

	ClipperLib::PolyTree polyTreeResultsIntersection;
	ClipperLib::PolyTree polyTreeResultsDiffs;

	Q_ASSERT(!surf.m_polygon.vertexes().empty());

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
			mainPoly[1+idxHole] = convertVec2DToClipperPath(holePoly.vertexes());
			bool orientationHolePoly = ClipperLib::Orientation(mainPoly[1+idxHole]);
			// Init PolyNode
			ClipperLib::PolyNode pnHole;
			pnHole.Contour = convertVec2DToClipperPath(holePoly.vertexes());
			pnMain.Childs.push_back(&pnHole);
		}
	}


	// set up clipper lib paths
	ClipperLib::Paths	otherPoly(2);
	ClipperLib::Path	&otherPolyClp = otherPoly[0];
	ClipperLib::Path	&otherHoleClp = otherPoly.back();

	// set up second polygon for clipper
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

		// Add back main intersection
		mainIntersections.push_back(ClippingPolygon());
		IBKMK::Polygon2D &poly = mainIntersections.back().m_polygon;

		if(poly.isValid())
			mainIntersections.back().m_area = poly.area();

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
		// const ClipperLib::Path &path = childNode->Contour;

		// Done to retain valid polygons for VICUS
		ClipperLib::Paths paths;
		ClipperLib::SimplifyPolygon(childNode->Contour, paths);

		for(const ClipperLib::Path &path : paths) {

			// Add back main intersection
			mainDiffs.push_back(ClippingPolygon());
			IBKMK::Polygon2D &poly = mainDiffs.back().m_polygon;

			// Convert back points
			poly.setVertexes(convertClipperPathToVec2D(path));

			// ToDo Stephan ist das richtig wenn das aktuelle diff-poly keine punkte hat dann einfach dieses zu überspringen
			// kann dann das nächste polygon ein loch von diesem sein?
			if(poly.vertexes().empty()){
				mainDiffs.pop_back();
				continue;
			}

			if(poly.isValid())
				mainDiffs.back().m_area = poly.area();

			for(ClipperLib::PolyNode *secondChild : childNode->Childs){
				if(!secondChild->IsHole())
					continue;
				IBKMK::Polygon2D polyHole;

				polyHole.setVertexes(convertClipperPathToVec2D(secondChild->Contour));
				mainDiffs.back().m_holePolygons.push_back(polyHole);
			}
		}
		// Should not be an hole
		Q_ASSERT(!childNode->IsHole());
	}
}
}
