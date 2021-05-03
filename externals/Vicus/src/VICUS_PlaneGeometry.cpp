#include "VICUS_PlaneGeometry.h"

#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <IBK_assert.h>
#include <IBK_Line.h>
#include <IBK_physics.h>

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


double PlaneGeometry::inclination() const {
	return std::acos(normal().m_z) / IBK::DEG2RAD;
}


double PlaneGeometry::orientation() const {
	double val = 90 - std::atan2(normal().m_y, normal().m_x) / IBK::DEG2RAD;
	if (val<0)
		val += 360;
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
	m_triangles.clear();
	m_triangleVertexes.clear();

	// only continue, if the polygon itself is valid
	if (!m_polygon.isValid())
		return;

	// We have special handling for type triangle and rectangle, but only if
	// we have no holes. This will safe some work for quite a lot of planes.
	std::vector<IBKMK::Vector3D> vertexes = m_polygon.vertexes();
	if (m_holes.empty() && m_polygon.type() != VICUS::Polygon3D::T_Polygon) {
		switch (m_polygon.type()) {

			case Polygon3D::T_Triangle :
				m_triangles.push_back( triangle_t(0, 1, 2) );
				break;

			case Polygon3D::T_Rectangle :
				m_triangles.push_back( triangle_t(0, 1, 2) );
				m_triangles.push_back( triangle_t(2, 3, 0) );
			break;

			default: ;
		}
		m_triangleVertexes.swap(vertexes);

		return; // done
	}


	// now the generic code

	// process all holes and check if they are valid (i.e. their polygons do not intersect our polygons
	std::vector<unsigned int> validPolygons;
	for (const Polygon2D & p2 : m_holes) {
		// check if any of the holes are invalid
		// TODO
	}

	// now populate global vertex vector and generate 2D polygon
	// we have then two vectors, one with the 3D vertexes, and one with the 2D vertexes inside the plane

	// first add the polygon points
	std::vector<IBKMK::Vector2D> points = m_polygon.polyline().vertexes();

	// Vector 'points' contains now the 2D coordinates for all 3D coordinates in 'vertexes' with same order.

	// now generate the edges for this polygon
	std::vector<std::pair<unsigned int, unsigned int> > edges;
	for (unsigned int i=0; i<points.size(); ++i) {
		edges.push_back(std::make_pair(i, (i+1) % points.size()));
	}
	edges.back().second = 0; // close the loop

	// add windows

	// loop all windows

	// for each vertex in window do:
	//  - check if vertex is already in vertex list, then re-use same vertex index,
	//    otherwise add vertex and get new index
	//  - add edge to this index

	IBKMK::Triangulation triangu;

	// IBK::point2D<double> is a IBKMK::Vector2D
	triangu.setPoints(reinterpret_cast< const std::vector<IBK::point2D<double> > & >(points), edges);

	for (auto tri : triangu.m_triangles) {
		// TODO : only add triangles if not degenerated (i.e. area = 0) (may happen in case of windows aligned to outer bounds)
		m_triangles.push_back(triangle_t(tri.i1, tri.i2, tri.i3));
	}

	// store vertexes for triangles
	m_triangleVertexes.swap(vertexes);
}


bool PlaneGeometry::intersectsLine(const IBKMK::Vector3D & p1, const IBKMK::Vector3D & d, IBKMK::Vector3D & intersectionPoint,
								   double & dist, bool hitBackfacingPlanes, bool endlessPlane) const
{
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

	// plane is endless - return intersection point and normalized distance t
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

	double x,y;
	if (!IBKMK::planeCoordinates(planeOffset, localX(), localY(), x0, x, y))
		return false;

	// test if point is in polygon
	// TODO : we need to test for individual triangles, since we now have holes
	//        or we need to check for holes first, and then for the enclosing polygons. Then
	//        this function should also indicate, which hole was clicked on (if any).
	if (IBKMK::pointInPolygon(m_polygon2D.vertexes(), IBK::point2D<double>(x,y) ) != -1) {
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



double PlaneGeometry::area() const {
	FUNCID(PlaneGeometry::area);
	if (!isValid())
		throw IBK::Exception(IBK::FormatString("Invalid polygon set."), FUNC_ID);

	return m_polygon2D.area();
}




} // namespace VICUS
