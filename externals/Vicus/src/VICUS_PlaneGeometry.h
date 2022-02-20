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

#ifndef VICUS_PlaneGeometryH
#define VICUS_PlaneGeometryH

#include <IBKMK_Vector3D.h>
#include <IBKMK_Polygon3D.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Polygon3D.h"
#include "VICUS_PlaneTriangulationData.h"

namespace VICUS {

/*! Class PlaneGeometry encapsulates the vertex data and plane type of a single plane
	in the geometrical model. It also includes subsurfaces and handles triangulation of
	the outer polygon (optionally with holes) and the triangulation of the subsurfaces.

	Also, it implements intersection tests (for picking).


*/
class PlaneGeometry {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Default constructor. */
	PlaneGeometry() {}

	/*! Initializing constructor.
		Vertexes a, b and c must be given in counter-clockwise order, so that (b-a) x (c-a) yields the normal vector of the plane.
	*/
	PlaneGeometry(IBKMK::Polygon2D::type_t t, const IBKMK::Vector3D & a, const IBKMK::Vector3D & b, const IBKMK::Vector3D & c);

	/*! A polygon is considered "fully valid" for painting and additing to the data structure, if
		it has enough vertexes and can be correctly triangulated (triangles not empty).
	*/
	bool isValid() const { return m_polygon.isValid() && !m_triangulationData.m_triangles.empty(); }

	/*! Return the inclination in Deg. 0° -> Roof; 90° -> Wall; 180° -> Floor. */
	double inclination(int digits = 1) const;

	/*! Return the orientation in Deg. 0° -> North; 90° -> East; 180° -> South; etc. */
	double orientation(int digits = 1) const;

	const IBKMK::Vector3D & normal() const { return m_polygon.normal(); }
	const IBKMK::Vector3D & localX() const { return m_polygon.localX(); }
	const IBKMK::Vector3D & localY() const { return m_polygon.localY(); }
	/*! Returns the offset point (origin of the plane's local coordinate system) */
	const IBKMK::Vector3D & offset() const { return m_polygon.vertexes()[0]; }

	/*! Adds a new 3D vertex.
		Calculates 2D plane coordinates and throws an exception, if vertex is out of plane.
	*/
	void addVertex(const IBKMK::Vector3D & v);

	/*! Removes the vertex at given location. */
	void removeVertex(unsigned int idx);

	/*! Tests if a line (with equation p = p1 + t * d) hits this plane. Returns true if
		intersection is found, and returns the normalized distance (t) between intersection point
		'intersectionPoint' and point p1.

		The optional argument hitBackfacingPlanes disables the front-facing check (if true).
		The optional argument endlessPlane disables the check if the intersection point
		lies within the plane (useful for getting intersections with, for example, the xy-plane).

		If plane contains holes, 'holeIndex' contains the index of the respective hole. If the plane
		does not have any holes or the opaque surface was clicked, holeIndex will be -1.
	*/
	bool intersectsLine(const IBKMK::Vector3D & p1,
						const IBKMK::Vector3D & d,
						IBKMK::Vector3D & intersectionPoint,
						double & dist, int & holeIndex,
						bool hitBackfacingPlanes = false,
						bool endlessPlane = false) const;

	/*! Returns the triangulation data for the opaque surface. */
	const PlaneTriangulationData & triangulationData() const { return m_triangulationData; }
	/*! Returns the triangulation data for each of the holes. */
	const std::vector<PlaneTriangulationData> & holeTriangulationData() const { return m_holeTriangulationData; }

	/*! Returns the stored polygon. */
	const Polygon3D & polygon() const { return m_polygon; }

	/*! Sets a new polygon and updates triangulation.
		Any holes previously set remain in the polygon. However, if the polygon now clips any
		of the holes, this will be detected in the triangulation and may lead to
		invalidation of holes.
		To clear/remove the holes, simply set an empty vector in setHoles().
	*/
	void setPolygon(const Polygon3D & polygon3D);

	/*! Returns the vector of holes (2D polygons in the plane of the polygon). */
	const std::vector<Polygon2D> & holes() const { return m_holes; }
	/*! Sets the vector of holes (2D polygons in the plane of the polygon). */
	void setHoles(const std::vector<Polygon2D> & holes);

	/*! Set outer polygon and holes together (needs only one call to triangulate()). */
	void setGeometry(const Polygon3D & polygon3D, const std::vector<Polygon2D> & holes);

	/*! Returns the 2D polygon (only if it exists) in the plane of the polygon. */
	const Polygon2D & polygon2D() const { return m_polygon.polyline(); }

	/*! Calculates surface area in m2. */
	double area(int digits = 1) const;

	/*! Calculates the center point of the surface/polygon */
	IBKMK::Vector3D centerPoint() const { return m_polygon.centerPoint(); }

private:

	// *** PRIVATE MEMBER FUNCTIONS ***

	/*! This function triangulates the geometry and populate the m_triangles and m_triangleVertexes vectors.
		This function is called whenever the polygon or the holes change.
	*/
	void triangulate();


	// *** PRIVATE MEMBER VARIABLES ***

	/*! Contains the information about the polygon that encloses this surface. */
	Polygon3D							m_polygon;

	/*! Polygons with holes/subsurfaces inside the polygon. */
	std::vector<Polygon2D>				m_holes;

	/*! Contains the vertex indexes for each triangle that the polygon is composed of.
		Includes only the triangles of the opaque surfaces without any holes
	*/
	PlaneTriangulationData				m_triangulationData;

	/*! Contains the triangulation data of each hole.
		Invalid hole definitions will yield empty triangle vectors.
	*/
	std::vector<PlaneTriangulationData>	m_holeTriangulationData;
};

} // namespace VICUS

#endif // VICUS_PlaneGeometryH
