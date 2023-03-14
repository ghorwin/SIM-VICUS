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

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Object.h"
#include "VICUS_Polygon3D.h"
#include "VICUS_Polygon2D.h"
#include "VICUS_PlaneTriangulationData.h"

namespace VICUS {

/*! Class PlaneGeometry encapsulates the vertex data and plane type of a single plane
	in the geometrical model. Its main purpose is the triangulation of
	the outer polygon (optionally with holes) and the triangulation of the subsurfaces and to provide
	said data for visualization. Also, it implements intersection tests (for picking).

	Since triangulation operations may be costly, the class implements lazy evaluation. Anything setting function
	that affects the computed triangulation just marks the PlaneGeometry as dirty. The triangulation is
	then computed on demand the triangulation data is accessed.

	Note: PlaneGeometry is a runtime-only class and not used in the VICUS::Project data model. Instead,
		  all data stored in PlaneGeometry is actually held in VICUS::Surface and its member variables.
*/

class PlaneGeometry {
public:

    /*! Struct for Holes- */
    struct Hole {

        Hole(unsigned int idObj, const VICUS::Polygon2D &hole, bool isChildSurface) :
            m_idObject(idObj),
            m_holeGeometry(hole),
            m_isChildSurface(isChildSurface)
        {}

		/*! ID to corresponding vicus hole object. */
        unsigned int            m_idObject = INVALID_ID;
        /*! Hole geometry. */
        VICUS::Polygon2D        m_holeGeometry;
        /*! Hole is part of an child surface. */
        bool                    m_isChildSurface;
    };


	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Default constructor. */
	PlaneGeometry() {}

	/*! Initializing constructor.
		Vertexes a, b and c must be given in counter-clockwise order, so that (b-a) x (c-a) yields the normal vector of the plane.
	*/
	PlaneGeometry(IBKMK::Polygon2D::type_t t, const IBKMK::Vector3D & a, const IBKMK::Vector3D & b, const IBKMK::Vector3D & c);

	/*! Convenience constructor, initializes plane geometry with Polygon3D. */
	PlaneGeometry(const IBKMK::Polygon3D & poly3D);

	/*! A polygon is considered "fully valid" for painting, if it has enough vertexes, a valid polygon and can be
		correctly triangulated (triangles not empty).
	*/
	bool isValid() const;

	/*! Return the inclination in Deg. 0° -> Roof; 90° -> Wall; 180° -> Floor. */
	double inclination(int digits = 1) const;

	/*! Return the orientation in Deg. 0° -> North; 90° -> East; 180° -> South; etc. */
	double orientation(int digits = 1) const;

	// Convenience query functions, using Polygon3D for data delivery

	const VICUS::Polygon3D & polygon3D() const { return m_polygon; }
	/*! Returns the offset point (origin of the plane's local coordinate system) */
	const IBKMK::Vector3D & offset() const { return m_polygon.vertexes()[0]; }
	const IBKMK::Vector3D & normal() const { return m_polygon.normal(); }
	const IBKMK::Vector3D & localX() const { return m_polygon.localX(); }
	const IBKMK::Vector3D & localY() const { return m_polygon.localY(); }

	/*! Calculates surface area in m2.
		Requires valid polygon, otherwise an exception is thrown.
	*/
    double area(int digits = 1) const;

	/*! Calculates the center point of the surface/polygon .
		Requires valid polygon, otherwise an exception is thrown.
	*/
	IBKMK::Vector3D centerPoint() const { return m_polygon.centerPoint(); }

	// Getter functions for stored geometry

	/*! Returns the 2D polygon (only if it exists) in the plane of the polygon. */
	const IBKMK::Polygon2D & polygon2D() const { return m_polygon.polyline(); }
	/*! Returns the vector of holes (2D polygons in the plane of the polygon). */
    const std::vector<Hole> & holes() const { return m_holes; }

	// Setter functions for plane geometry

	/*! Sets the outer polygon alone. */
	void setPolygon(const Polygon3D & polygon3D);

	/*! Sets the vector of holes (2D polygons in the plane of the polygon). */
    void setHoles(const std::vector<Hole> &holes);

	/*! Set outer polygon and holes together (needs only one call to triangulate()). */
    void setGeometry(const Polygon3D & polygon3D, const std::vector<Hole> &holes);

	/*! Flips the normal vector of the geometry. Does not require update of triangulation.
		Requires valid polygon, otherwise an exception is thrown.
	*/
	void flip();

	void changeOrigin(unsigned int idx);

	// Getter functions for triangulation data (lazy evaluation)

	/*! Returns the triangulation data for the opaque surface (excluding holes). */
	const PlaneTriangulationData & triangulationData() const;
	/*! Returns the triangulation data for the opaque surface (including holes, as if there were no holes). */
	const PlaneTriangulationData & triangulationDataWithoutHoles() const;
	/*! Returns the triangulation data for each of the holes. */
	const std::vector<PlaneTriangulationData> & holeTriangulationData() const;

	// Utility functions

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

private:

	// *** PRIVATE MEMBER FUNCTIONS ***

	/*! This function triangulates the geometry and populate the m_triangles and m_triangleVertexes vectors.
		This function is called whenever the polygon or the holes change.

		\note This function is const, because it is called from const functions.
	*/
	void triangulate() const;


	// *** PRIVATE MEMBER VARIABLES ***

	/*! Contains the information about the polygon that encloses this surface. */
	Polygon3D									m_polygon;

	/*! Polygons with holes/subsurfaces inside the polygon. */
    std::vector<Hole>                           m_holes;

	/*! Contains the vertex indexes for each triangle that the polygon is composed of.
		Includes only the triangles of the opaque surfaces without any holes
	*/
	mutable PlaneTriangulationData				m_triangulationData;

	/*! Contains the vertex indexes for each triangle that the polygon is composed of.
		Includes only the triangles of the opaque surfaces without any holes
	*/
	mutable PlaneTriangulationData				m_triangulationDataWithoutHoles;

	/*! Contains the triangulation data of each hole.
		Invalid hole definitions will yield empty triangle vectors.
	*/
	mutable std::vector<PlaneTriangulationData>	m_holeTriangulationData;

	/*! Dirty flag marks the triangulation data as outdated and is set whenever plane geometry data is modified. */
	mutable bool								m_dirty = true;
};

} // namespace VICUS

#endif // VICUS_PlaneGeometryH
