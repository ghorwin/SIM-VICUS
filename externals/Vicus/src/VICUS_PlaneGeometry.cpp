/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

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

#include "VICUS_PlaneGeometry.h"

#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <IBK_assert.h>
#include <IBK_Line.h>
#include <IBK_physics.h>
#include <IBK_math.h>

#include <IBKMK_Triangulation.h>
#include <IBKMK_3DCalculations.h>
#include <IBKMK_2DCalculations.h>

#include <NANDRAD_Utilities.h>

#include <VICUS_Constants.h>
#include <VICUS_KeywordList.h>

#include <QPolygonF>
#include <QVector2D>
#include <QQuaternion>
#include <QLine>

#include <tinyxml.h>

namespace VICUS {



// *** PlaneGeometry ***

PlaneGeometry::PlaneGeometry(Polygon3D::type_t t,
							 const IBKMK::Vector3D & a, const IBKMK::Vector3D & b, const IBKMK::Vector3D & c) :
	m_polygon(t, a, b, c)
{
	triangulate();
}


double PlaneGeometry::inclination(int digits) const {
	double inc = std::acos(normal().m_z) / IBK::DEG2RAD;
	inc = std::floor(inc*IBK::f_pow10(digits))/IBK::f_pow10(digits);
	return inc;
}


double PlaneGeometry::orientation(int digits) const {
	double val = 90 - std::atan2(normal().m_y, normal().m_x) / IBK::DEG2RAD;
	if (val<0)
		val += 360;

	// round value to n digits

	val = std::floor(val*IBK::f_pow10(digits))/IBK::f_pow10(digits);

	return  val;
}


void PlaneGeometry::addVertex(const IBKMK::Vector3D & v) {
	m_polygon.addVertex(v);
	triangulate(); // if we have a triangle/rectangle, this is detected here
}


void PlaneGeometry::removeVertex(unsigned int idx) {
	m_polygon.removeVertex(idx);
	triangulate(); // if we have a triangle/rectangle, this is detected here
}

// TODO : Dirk, move the 2 functions below to IBKMK_2DCalculations and port them to
//        use Vector2D or point2D

/*!
	Copyright 2000 softSurfer, 2012 Dan Sunday
	This code may be freely used and modified for any purpose
	providing that this copyright notice is included with it.
	SoftSurfer makes no warranty for this code, and cannot be held
	liable for any real or imagined damage resulting from its use.
	Users of this code must verify correctness for their application.

	 isLeft(): tests if a point is Left|On|Right of an infinite line.
		Input:  three points P0, P1, and P2
		Return: >0 for P2 left of the line through P0 and P1
				=0 for P2  on the line
				<0 for P2  right of the line
		See: Algorithm 1 "Area of Triangles and Polygons"
	*/


inline int isLeft( QPoint P0, QPoint P1, QPoint P2 )
{
	return ( (P1.x() - P0.x()) * (P2.y() - P0.y())
			- (P2.x() -  P0.x()) * (P1.y() - P0.y()) );
}

/*!
	URL: http://geomalgorithms.com/a03-_inclusion.html

	Copyright 2000 softSurfer, 2012 Dan Sunday
	This code may be freely used and modified for any purpose
	providing that this copyright notice is included with it.
	SoftSurfer makes no warranty for this code, and cannot be held
	liable for any real or imagined damage resulting from its use.
	Users of this code must verify correctness for their application.

	wn_PnPoly(): winding number test for a point in a polygon
	  Input:   P = a point,
			   V[] = vertex points of a polygon V[n+1] with V[n]=V[0]
	  Return:  wn = the winding number (=0 only when P is outside)
	*/
int wn_PnPoly( QPoint P, QPoint *V, int n )
{
	int wn = 0;											// the  winding number counter

	// loop through all edges of the polygon
	for (int i=0; i<n; i++) {							// edge from V[i] to  V[i+1]
		if (V[i].y() <= P.y()) {						// start y <= P.y
			if (V[i+1].y()  > P.y())					// an upward crossing
				 if (isLeft( V[i], V[i+1], P) > 0)		// P left of  edge
					 ++wn;								// have  a valid up intersect
		}
		else {											// start y > P.y (no test needed)
			if (V[i+1].y()  <= P.y())					// a downward crossing
				 if (isLeft( V[i], V[i+1], P) < 0)		// P right of  edge
					 --wn;								// have  a valid down intersect
		}
	}
	return wn;
}


void PlaneGeometry::triangulate() {
	m_triangulationData.clear();
	m_holeTriangulationData.clear();

	// only continue, if the polygon itself is valid
	if (!m_polygon.isValid())
		return;

	// We have special handling for type triangle and rectangle, but only if
	// we have no holes. This will save some work for quite a lot of planes.
	std::vector<IBKMK::Vector3D> vertexes = m_polygon.vertexes();
	if (m_holes.empty() && m_polygon.type() != VICUS::Polygon3D::T_Polygon) {
		switch (m_polygon.type()) {

			case Polygon3D::T_Triangle :
				m_triangulationData.m_triangles.push_back( IBKMK::Triangulation::triangle_t(0, 1, 2) );
				break;

			case Polygon3D::T_Rectangle :
				m_triangulationData.m_triangles.push_back( IBKMK::Triangulation::triangle_t(0, 1, 2) );
				m_triangulationData.m_triangles.push_back( IBKMK::Triangulation::triangle_t(2, 3, 0) );
			break;

			default: ;
		}
		m_triangulationData.m_vertexes.swap(vertexes);
		m_triangulationData.m_normal = m_polygon.normal(); // cache normal for easy access

		return; // done
	}


	// now the generic code

	const std::vector<IBKMK::Vector2D> &parentPoly = m_polygon.polyline().vertexes();
	// process all holes and check if they are valid (i.e. their polygons do not intersect our polygons

	// here we store the state of the sub-surface polygon: 0 - invalid, 1 - partially valid, 2 - completely valid
	// polygons with status 1 are drawn but not used in triangulation of polygon3D
	std::vector<unsigned int> polygonStatus(m_holes.size(), 0);
	for (unsigned int i=0; i<m_holes.size(); ++i) {
		const Polygon2D & p2 = m_holes[i];
		const std::vector<IBKMK::Vector2D> &subSurfacePoly = p2.vertexes();

		// check if any of the holes are invalid
		// TODO : Dirk, invalide Polygone einfach ignorieren und nicht zeichnen? Oder als "outline" zeichnen?
		//        Damit man diese "invaliden" Polygonflächen noch anfassen kann und vielleicht zurück in die
		//        Konstruktion schieben kann, müsste man diese trotzdem triangulieren. Vorschlag:
		//        Unterscheiden zwischen komplett ungültig (verwunden, nur 2 nicht-kollinieare Punkte, ...)
		//        und korrigierbar falsch ("außerhalb des Polygons" oder "überlappend mit anderem Fenster").
		//        Letztere könnte man zunächst auch noch zeichnen, sollte dann aber den "invalid" Zustand irgendwie
		//        visualisiert bekommen (und beim NANDRAD -Export vielleicht als Fehler angezeigt bekommen?).


		// skip invalid polygons alltogether
		if (!p2.isValid()) {
			polygonStatus[i] = 0;
			continue;
		}


		// First check: test all vertexes of polygon;
		// - if outside outer polygon, sub-surface is (partially) outside polygon

		bool valid = true;
		unsigned int pointCountHole = subSurfacePoly.size();
		for(unsigned int j=0; j<pointCountHole; ++j) {
			if (IBKMK::pointInPolygon(parentPoly, subSurfacePoly[j]) == -1){
				valid = false;
				break;
			}
		}

		if (!valid) {
			polygonStatus[i] = 1;
			continue;
		}

		// Second check: test intersections of all sub-surface polygon edges with edges of outer polygon
		for(unsigned int j=0; j<pointCountHole; ++j){
			const IBKMK::Vector2D &point1 = subSurfacePoly[j];
			const IBKMK::Vector2D &point2 = subSurfacePoly[(j+1) % pointCountHole];
			IBKMK::Vector2D intersectP;
			if (IBKMK::intersectsLine2D(parentPoly, point1, point2, intersectP)) {
				// if intersection point is close to first or second point of line, polygon is ok
				if (! (IBK::nearly_equal<3>(intersectP.distanceTo(point1),0) ||
					   IBK::nearly_equal<3>(intersectP.distanceTo(point2),0)    )  )
				{
					valid = false;
					break;
				}
			}

		}
		if (!valid) {
			polygonStatus[i] = 1;
			continue;
		}

		// TODO Dirk, check intersections with other sub-surfaces

		// all checks done
		polygonStatus[i] = 2;
	}

	// now populate global vertex vector and generate 2D polygon
	// we have then two vectors, one with the 3D vertexes, and one with the 2D vertexes inside the plane

	// first add the polygon points
	std::vector<IBKMK::Vector2D> points = m_polygon.polyline().vertexes();

	// Vector 'points' contains now the 2D coordinates for all 3D coordinates in 'vertexes' with same order.

	// now generate the edges for this polygon
	std::vector<std::pair<unsigned int, unsigned int> > edges;
	for (unsigned int i=0; i<points.size(); ++i)
		edges.push_back(std::make_pair(i, (i+1) % points.size()));


	// we now have the points of the outer polygon in vector 'points' and the outer edges defined in 'edges'


	// now process holes/windows
	m_holeTriangulationData.resize(m_holes.size()); // create an (empty) data structure for each hole

	// loop all holes/windows
	for (unsigned int holeIdx = 0; holeIdx < polygonStatus.size(); ++holeIdx) {
		// skip all entirely invalid polygons
		if (polygonStatus[holeIdx] == 0)
			continue;

		const Polygon2D & p2 = m_holes[holeIdx];

		std::vector<unsigned int> vertexIndexes; // mapping table, relates hole index to global index in 'points' vector
		// process all vertexes
		const std::vector<IBKMK::Vector2D> & holePoints = p2.vertexes();
		for (const IBKMK::Vector2D & v : holePoints) {
			// for each vertex in window do:
			//  - check if vertex is already in vertex list, then re-use same vertex index,
			//    otherwise add vertex and get new index
			unsigned int j=0;
			for (;j<points.size();++j)
				if (points[j] == v) // TODO : here we might rather use some fuzzy comparison to avoid very tiny triangles
					break;
			// store index (either of existing vertex or next vertex to be added)
			vertexIndexes.push_back(j);
			if (j == points.size()) {
				points.push_back(v);
				// compute the matching 3D vertex and add to list of vertexes
				IBKMK::Vector3D v3 = offset() + localX()*v.m_x + localY()*v.m_y;
				vertexes.push_back(v3);
			}
		}

		// now the 'points' vector holds vertexes of the outer polygon and the holes

		// add edges
		std::vector<std::pair<unsigned int, unsigned int> > holeOnlyEdges;
		for (unsigned int i=0, vertexCount = p2.vertexes().size(); i<vertexCount; ++i) {
			unsigned int i1 = vertexIndexes[i];
			unsigned int i2 = vertexIndexes[(i+1) % vertexCount];
			// add edge to global edge list (for outer polygon), but only, if polygon is entirely valid
			if (polygonStatus[holeIdx] == 2)
				edges.push_back(std::make_pair(i1, i2));
			// add edge to vector with only hole edges
			holeOnlyEdges.push_back(std::make_pair(i1, i2));
		}

		// create inverse vertex lookup map
		std::vector<unsigned int> inverseVertexIndexes(points.size(), (unsigned int)-1);
		for (unsigned int i=0; i<vertexIndexes.size();++i)
			inverseVertexIndexes[vertexIndexes[i]] = i;

		// Example: outer polygon is defined through vertexes 0...3
		//          hole is defined through vertexes 4...7
		//          'points' holds vertexes 0...7
		//          'edges' holds all edges and uses all vertexes
		//          'holeOnlyEdges' holds only edges of hole: 4-5, 5-6, 6-7, 7-4
		//          'vertexIndexes' maps hole-vertex-index to global index:
		//            vertexIndexes[0] = 4
		//            vertexIndexes[1] = 5
		//            vertexIndexes[2] = 6
		//            vertexIndexes[3] = 7
		//          'inverseVertexIndexes' maps global point index to hole-vertex-index:
		//            inverseVertexIndexes[0] = -1 (unused)
		//            inverseVertexIndexes[1] = -1
		//            inverseVertexIndexes[2] = -1
		//            inverseVertexIndexes[3] = -1
		//            inverseVertexIndexes[4] = 0
		//            inverseVertexIndexes[5] = 1
		//            inverseVertexIndexes[6] = 2
		//            inverseVertexIndexes[7] = 3

		// now do the triangulation of the window alone
		IBKMK::Triangulation triangu;

		// IBK::point2D<double> is a IBKMK::Vector2D
		// Note: we give the global points vector
		triangu.setPoints(reinterpret_cast< const std::vector<IBK::point2D<double> > & >(points), holeOnlyEdges);
		// and copy the triangle data and remap the points
		std::vector<IBKMK::Vector3D> holeVertexes3D(holePoints.size());
		for (auto tri : triangu.m_triangles) {
			// TODO Dirk: only add triangles if not degenerated (i.e. area = 0) (may happen in case of windows aligned to outer bounds)
			IBKMK::Triangulation::triangle_t remappedTriangle;
			// remap the vertexes
			remappedTriangle.i1 = inverseVertexIndexes[tri.i1]; // index of local hole vertex
			holeVertexes3D[remappedTriangle.i1] = vertexes[tri.i1];

			remappedTriangle.i2 = inverseVertexIndexes[tri.i2]; // index of local hole vertex
			holeVertexes3D[remappedTriangle.i2] = vertexes[tri.i2];

			remappedTriangle.i3 = inverseVertexIndexes[tri.i3]; // index of local hole vertex
			holeVertexes3D[remappedTriangle.i3] = vertexes[tri.i3];

			m_holeTriangulationData[holeIdx].m_triangles.push_back(remappedTriangle);
		}
		m_holeTriangulationData[holeIdx].m_vertexes.swap(holeVertexes3D);
		m_holeTriangulationData[holeIdx].m_normal = m_polygon.normal(); // cache normal for easy access
	}

	IBKMK::Triangulation triangu;
	// Note: IBK::point2D<double> is a IBKMK::Vector2D
	triangu.setPoints(reinterpret_cast< const std::vector<IBK::point2D<double> > & >(points), edges);

	for (auto tri : triangu.m_triangles) {
		// TODO Dirk: only add triangles if not degenerated (i.e. area = 0) (may happen in case of windows aligned to outer bounds)
		m_triangulationData.m_triangles.push_back(IBKMK::Triangulation::triangle_t(tri.i1, tri.i2, tri.i3));
	}

	// store vertexes for triangles
	m_triangulationData.m_vertexes.swap(vertexes);
	m_triangulationData.m_normal = m_polygon.normal(); // cache normal for easy access
}


bool PlaneGeometry::intersectsLine(const IBKMK::Vector3D & p1, const IBKMK::Vector3D & d, IBKMK::Vector3D & intersectionPoint,
								   double & dist, int & holeIndex, bool hitBackfacingPlanes, bool endlessPlane) const
{
	holeIndex = -1;
	// We need to guard against invalid geometry
	if (!isValid())
		return false;

	// first the normal test

	double d_dot_normal = d.scalarProduct(normal());
	double angle = d_dot_normal/d.magnitude();
	// line parallel to plane? no intersection
	if (angle < 1e-8 && angle > -1e-8)
		return false;

	// Condition 1: same direction of normal vectors?
	if (!hitBackfacingPlanes && angle >= 0)
		return false; // no intersection possible

	const IBKMK::Vector3D & planeOffset = offset();
	double t = (planeOffset - p1).scalarProduct(normal()) / d_dot_normal;

	// Condition 2: outside viewing range?
	if (t < 0 || t > 1)
		return false;

	// now determine location on plane
	IBKMK::Vector3D x0 = p1 + t*d;

	// plane is endless - return intersection point and normalized distance t (no hole checking here!)
	if (endlessPlane) {
		intersectionPoint = x0;
		dist = t;
		return true;
	}

	// test if intersection point is inside our plane
	// we have a specialized variant for triangles and rectangles, but only if there are no holes
	if (m_holes.empty() && m_polygon.type() != Polygon3D::T_Polygon) {

		switch (m_polygon.type()) {
			case Polygon3D::T_Triangle :
			case Polygon3D::T_Rectangle : {

				// we have three possible ways to get the intersection point, try them all until we succeed
				const IBKMK::Vector3D & a = m_polygon.vertexes()[1] - m_polygon.vertexes()[0];
				const IBKMK::Vector3D & b = m_polygon.vertexes().back() - m_polygon.vertexes()[0];
				double x,y;
				if (!IBKMK::planeCoordinates(planeOffset, a, b, x0, x, y))
					return false;

				if (m_polygon.type() == Polygon3D::T_Triangle && x >= 0 && x+y <= 1 && y >= 0) {
					intersectionPoint = x0;
					dist = t;
					return true;
				}
				else if (m_polygon.type() == Polygon3D::T_Rectangle && x >= 0 && x <= 1 && y >= 0 && y <= 1) {
					intersectionPoint = x0;
					dist = t;
					return true;
				}

			} break;
			default: ;
		}
		return false;
	}

	// compute 2D coordinates in local plane - if that fails, bail out (can happen when line is parallel to plane)
	double x,y;
	if (!IBKMK::planeCoordinates(planeOffset, localX(), localY(), x0, x, y))
		return false;

	IBK::point2D<double> intersectionPoint2D(x,y);

	// first check if we hit a hole
	for (unsigned int j=0; j<m_holes.size(); ++j) {
		// - if hole is valid, check if we are inside the polygon
		if (m_holes[j].isValid()) {
			// run point in polygon algorithm
			if (IBKMK::pointInPolygon(m_holes[j].vertexes(), intersectionPoint2D) != -1) {
				holeIndex = (int)j;
				intersectionPoint = x0;
				dist = t;
				return true;
			}
		}
	}

	// otherwise we might still hit the rest of the polygon
	if (IBKMK::pointInPolygon(polygon2D().vertexes(), intersectionPoint2D ) != -1) {
		dist = t;
		intersectionPoint = x0;
		return true;
	}
	else
		return false;
}


void PlaneGeometry::setPolygon(const Polygon3D & polygon3D) {
	m_polygon = polygon3D;
	triangulate();
}


void PlaneGeometry::setHoles(const std::vector<Polygon2D> & holes) {
	if (m_holes.empty() && holes.empty())
		return;
	m_holes = holes;
	triangulate();
}


void PlaneGeometry::setGeometry(const Polygon3D & polygon3D, const std::vector<Polygon2D> & holes) {
	m_holes = holes;
	m_polygon = polygon3D;
	triangulate();
}


double PlaneGeometry::area() const {
	FUNCID(PlaneGeometry::area);
	if (!isValid())
		throw IBK::Exception(IBK::FormatString("Invalid polygon set."), FUNC_ID);

	return polygon2D().area();
}


} // namespace VICUS
