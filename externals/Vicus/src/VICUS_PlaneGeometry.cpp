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

PlaneGeometry::PlaneGeometry(IBKMK::Polygon2D::type_t t, const IBKMK::Vector3D & a, const IBKMK::Vector3D & b, const IBKMK::Vector3D & c) {
	// use special construction function from IBKMK::Polygon3D
	IBKMK::Polygon3D p(t, a, b, c);
	// if polygon is valid, transfer data
	if (p.isValid()) {
		// a valid polygon has at least 1 vertex and valid normal and localX vectors
		// TODO : since VICUS::Polygon3D _IS A_ IBKMK::Polygon3D we could just use assignment with a cast here.
		m_polygon = VICUS::Polygon3D((const VICUS::Polygon2D&)p.polyline(), p.normal(), p.localX(), p.vertexes()[0]);
		m_dirty = true;
	}
}


PlaneGeometry::PlaneGeometry(const IBKMK::Polygon3D & poly3D) {
	m_polygon = VICUS::Polygon3D(poly3D);
	m_dirty = true;
}


bool PlaneGeometry::isValid() const {
	if (!m_polygon.isValid())
		return false;
	if (m_dirty)
		triangulate();

	return !m_triangulationData.m_triangles.empty();
}


double PlaneGeometry::inclination(int digits) const {
	double inc = std::acos(normal().m_z) / IBK::DEG2RAD;

	inc = std::round(inc*IBK::f_pow10(digits))/IBK::f_pow10(digits);
	return inc;
}


double PlaneGeometry::orientation(int digits) const {
	double val = 90 - std::atan2(normal().m_y, normal().m_x) / IBK::DEG2RAD;
	if (val<0)
		val += 360;

	// round value to n digits
	val = std::round(val*IBK::f_pow10(digits))/IBK::f_pow10(digits);

    return  val;
}

double PlaneGeometry::area(int digits) const {
    double area = 0.0;
    for(const VICUS::PlaneGeometry::Hole &h : m_holes) {
        if(h.m_isChildSurface)
            area -= h.m_holeGeometry.area(digits);
    }
    return area + m_polygon.polyline().area(digits);
}


void PlaneGeometry::triangulate() const {
	m_triangulationData.clear();
	m_triangulationDataWithoutHoles.clear();
	m_holeTriangulationData.clear();
	m_dirty = false;

	// only continue, if the polygon itself is valid
	if (!m_polygon.isValid())
		return;

	// Create a copy of the vertex error of the outer polygon (we are going to add vertexes of the holes later on)
	// Note: do not access m_vertexes herea as we may regenerate the vertexes.
	std::vector<IBKMK::Vector3D> vertexes = m_polygon.vertexes();

	// We have special handling for type triangle and rectangle, but only if
	// we have no holes. This will save some work for quite a lot of planes.
	if (m_holes.empty() && m_polygon.type() != VICUS::Polygon2D::T_Polygon) {
		switch (m_polygon.type()) {

			case Polygon2D::T_Triangle :
				m_triangulationData.m_triangles.push_back( IBKMK::Triangulation::triangle_t(0, 1, 2) );
				break;

			case Polygon2D::T_Rectangle :
				m_triangulationData.m_triangles.push_back( IBKMK::Triangulation::triangle_t(0, 1, 2) );
				m_triangulationData.m_triangles.push_back( IBKMK::Triangulation::triangle_t(2, 3, 0) );
			break;

			default: ;
		}
		m_triangulationData.m_vertexes.swap(vertexes);
		m_triangulationData.m_normal = m_polygon.normal(); // cache normal for easy access

		// no holes, copy data
		m_triangulationDataWithoutHoles.m_vertexes = m_triangulationData.m_vertexes;
		m_triangulationDataWithoutHoles.m_normal = m_polygon.normal();

		return; // done
	}


	// now the generic code

	const std::vector<IBKMK::Vector2D> &parentPoly = m_polygon.polyline().vertexes();
	// process all holes and check if they are valid (i.e. their polygons do not intersect our polygons

	// here we store the state of the sub-surface polygon: 0 - invalid, 1 - partially valid, 2 - completely valid
	// polygons with status 1 are drawn but not used in triangulation of polygon3D
	std::vector<unsigned int> polygonStatus(m_holes.size(), 0);
	for (unsigned int i=0; i<m_holes.size(); ++i) {
        const Polygon2D & p2 = m_holes[i].m_holeGeometry;
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
		for (unsigned int j=0; j<pointCountHole; ++j) {
			if (IBKMK::pointInPolygon(parentPoly, subSurfacePoly[j]) == -1) {
				valid = false;
				break;
			}
		}

		if (!valid) {
			polygonStatus[i] = 1;
			continue;
		}

		// Second check: test intersections of all sub-surface polygon edges with edges of outer polygon
		for (unsigned int j=0; j<pointCountHole; ++j){
			const IBKMK::Vector2D &point1 = subSurfacePoly[j];
			const IBKMK::Vector2D &point2 = subSurfacePoly[(j+1) % pointCountHole];
			IBKMK::Vector2D intersectP;
			if (IBKMK::intersectsLine2D(parentPoly, point1, point2, intersectP)) {
				// if intersection point is close to first or second point of line, polygon is ok
				const double EPS = 1e-4;
				if (!  (intersectP.distanceTo(point1) < EPS ||  intersectP.distanceTo(point2) < EPS)    )
				{
					// polygon is also ok, if intersection point is "on" the outer polygon
					if (IBKMK::pointInPolygon(parentPoly, subSurfacePoly[j]) != 0) {
						valid = false;
						break;
					}
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
		// skip all invalid polygons; polygons partially or fully outside the outer polygon
		// are still being triangulated (and show), but they points are not used for the triangulation
		// of the outer polygon with holes
		if (polygonStatus[holeIdx] == 0)
			continue;

        const Polygon2D & p2 = m_holes[holeIdx].m_holeGeometry; // polygon of currently processed hole

		std::vector<unsigned int> vertexIndexes; // mapping table, relates hole index to global index in 'points' vector
		// process all vertexes and compose vector with 2D and 3D coordinates of the hole polygon
		std::vector<IBKMK::Vector2D> matchedHolePoints;
		std::vector<IBKMK::Vector3D> matchedHolePoints3D;
		const std::vector<IBKMK::Vector2D> & holePoints = p2.vertexes();
		for (const IBKMK::Vector2D & v : holePoints) {
			// for each vertex in window do:
			//  - check if vertex is already in vertex list, then re-use same vertex index,
			//    otherwise add vertex and get new index
			unsigned int j=0;
			for (; j<points.size(); ++j)
				if (points[j].similar(v, 1e-6))
					break;
			// store index (either of existing vertex or next vertex to be added)
			vertexIndexes.push_back(j);
			if (j == points.size()) {
				points.push_back(v);
				// compute the matching 3D vertex and add to list of vertexes
				IBKMK::Vector3D v3 = offset() + localX()*v.m_x + localY()*v.m_y;
				vertexes.push_back(v3);
			}
			matchedHolePoints.push_back(points[j]);
			matchedHolePoints3D.push_back(vertexes[j]);
		}

		// now the 'points' vector holds vertexes of the outer polygon and all the holes

//		IBK::IBK_Message("Edges\n", IBK::MSG_PROGRESS, "");
		// add edges
		std::vector<std::pair<unsigned int, unsigned int> > holeOnlyEdges;
		for (unsigned int i=0, vertexCount = p2.vertexes().size(); i<vertexCount; ++i) {
			unsigned int i1 = vertexIndexes[i];
			unsigned int i2 = vertexIndexes[(i+1) % vertexCount];
			// add edge to global edge list (for outer polygon), but only, if polygon is entirely valid
			if (polygonStatus[holeIdx] == 2) {
				edges.push_back(std::make_pair(i1, i2));
	//			IBK::IBK_Message(IBK::FormatString("   edges[%1]    = (%2, %3)\n").arg(i).arg(i1).arg(i2), IBK::MSG_PROGRESS, "");
			}

			// add edge to vector with only hole edges (numbered from 0...vertexCount-1)
			holeOnlyEdges.push_back(std::make_pair(i, (i+1) % vertexCount));
		}

		// Example: outer polygon is defined through vertexes 0...3
		//          hole is defined through vertexes 4...7
		//          'points' holds vertexes 0...7
		//          'edges' holds all edges (of outer polygon and inner polyons) and uses all vertexes
		//          'holeOnlyEdges' holds only edges of hole: 0-1, 1-2, 2-3, 3-0
		//          'vertexIndexes' maps hole-vertex-index to global index:
		//            vertexIndexes[0] = 4
		//            vertexIndexes[1] = 5
		//            vertexIndexes[2] = 6
		//            vertexIndexes[3] = 7

		// now do the triangulation of hole alone
		IBKMK::Triangulation triangu;

		// Note: we give the global points vector and all edgs of all holes
		//       IBK::point2D<double> is a IBKMK::Vector2D
		triangu.setPoints(reinterpret_cast< const std::vector<IBK::point2D<double> > & >(matchedHolePoints), holeOnlyEdges);

		// and copy the triangle data and remap the points
		for (auto tri : triangu.m_triangles) {
			if (tri.isDegenerated()) // protect against -1 vertex indexes
				continue;
			m_holeTriangulationData[holeIdx].m_triangles.push_back(tri);
		}
		m_holeTriangulationData[holeIdx].m_vertexes.swap(matchedHolePoints3D);
		m_holeTriangulationData[holeIdx].m_normal = m_polygon.normal(); // cache normal for easy access
	}

	// now generate the triangulation data for the entire surface, without subsurface polygons

#if 0
	for (auto p : points)
		IBK::IBK_Message(IBK::FormatString("	points.push_back( IBK::point2D<double>(%1,%2));\n")
						 .arg(p.m_x).arg(p.m_y), IBK::MSG_PROGRESS, "");
	for (auto e : edges)
		IBK::IBK_Message(IBK::FormatString("	edges.push_back(std::make_pair(%1, %2));\n").arg(e.first).arg(e.second), IBK::MSG_PROGRESS, "");
#endif

	IBKMK::Triangulation triangu;
	// Note: IBK::point2D<double> is a IBKMK::Vector2D
	triangu.setPoints(reinterpret_cast< const std::vector<IBK::point2D<double> > & >(points), edges);

//	IBK::IBK_Message("Triangulation\n", IBK::MSG_PROGRESS, "");
	for (auto tri : triangu.m_triangles) {
		// skip degenerated triangles
		if (tri.isDegenerated())
			continue;
		m_triangulationData.m_triangles.push_back(tri);
//		IBK::IBK_Message(IBK::FormatString("%1, %2, %3\n").arg(tri.i1).arg(tri.i2).arg(tri.i3), IBK::MSG_PROGRESS, "");
	}

	// store vertexes for triangles
	m_triangulationData.m_vertexes.swap(vertexes);
	m_triangulationData.m_normal = m_polygon.normal(); // cache normal for easy access

	Q_ASSERT(m_holes.size() == m_holeTriangulationData.size());
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
	if (m_holes.empty() && m_polygon.type() != Polygon2D::T_Polygon) {

		switch (m_polygon.type()) {
			case Polygon2D::T_Triangle :
			case Polygon2D::T_Rectangle : {

				// we have three possible ways to get the intersection point, try them all until we succeed
				const IBKMK::Vector3D & a = m_polygon.vertexes()[1] - m_polygon.vertexes()[0];
				const IBKMK::Vector3D & b = m_polygon.vertexes().back() - m_polygon.vertexes()[0];
				double x,y;
				if (!IBKMK::planeCoordinates(planeOffset, a, b, x0, x, y, VICUS_PLANE_PROJECTION_TOLERANCE))
					return false;

				if (m_polygon.type() == Polygon2D::T_Triangle && x >= 0 && x+y <= 1 && y >= 0) {
					intersectionPoint = x0;
					dist = t;
					return true;
				}
				else if (m_polygon.type() == Polygon2D::T_Rectangle && x >= 0 && x <= 1 && y >= 0 && y <= 1) {
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
	if (!IBKMK::planeCoordinates(planeOffset, localX(), localY(), x0, x, y, VICUS_PLANE_PROJECTION_TOLERANCE))
		return false;

	IBK::point2D<double> intersectionPoint2D(x,y);

	// first check if we hit a hole
	for (unsigned int j=0; j<m_holes.size(); ++j) {
		// - if hole is valid, check if we are inside the polygon
        if (m_holes[j].m_holeGeometry.isValid()) {
			// run point in polygon algorithm
            if (IBKMK::pointInPolygon(m_holes[j].m_holeGeometry.vertexes(), intersectionPoint2D) != -1) {
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
	m_dirty = true;
}


void PlaneGeometry::setHoles(const std::vector<Hole> & holes) {
	if (m_holes.empty() && holes.empty())
		return;
	m_holes = holes;
	m_dirty = true;
}


void PlaneGeometry::setGeometry(const Polygon3D & polygon3D, const std::vector<Hole> & holes) {
	m_holes = holes;
	m_polygon = polygon3D;
	m_dirty = true;
}


void PlaneGeometry::flip() {
	// flip the polygon first
	m_polygon.flip();
	// since localX and localY axes have been swapped, we need to swap all hole coordinates as well
    for (PlaneGeometry::Hole & h : m_holes) {
        std::vector<IBKMK::Vector2D>		vertexes2D = h.m_holeGeometry.vertexes();
		for (IBKMK::Vector2D & v : vertexes2D)
			std::swap(v.m_x, v.m_y);
        h.m_holeGeometry.setVertexes(vertexes2D);
	}
}

void PlaneGeometry::changeOrigin(unsigned int idx) {
	const std::vector<IBKMK::Vector3D> &verts = m_polygon.vertexes();

	if(verts.size()<3 || idx > verts.size()-1)
		return;

	std::vector<IBKMK::Vector3D> newVerts;
	newVerts.insert(newVerts.begin(), verts.begin() + idx, verts.end());
	newVerts.insert(newVerts.end(), verts.begin(), verts.begin() + idx);

	m_polygon.setVertexes(newVerts);
}


const PlaneTriangulationData & PlaneGeometry::triangulationData() const {
	if (m_dirty)
		triangulate();
	return m_triangulationData;
}


const PlaneTriangulationData & PlaneGeometry::triangulationDataWithoutHoles() const {
	if (m_dirty)
		triangulate();
	return m_triangulationDataWithoutHoles;
}


const std::vector<PlaneTriangulationData> & PlaneGeometry::holeTriangulationData() const {
	if (m_dirty)
		triangulate();
	return m_holeTriangulationData;
}


} // namespace VICUS
