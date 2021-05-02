#ifndef VICUS_PlaneGeometryH
#define VICUS_PlaneGeometryH

#include <IBKMK_Vector3D.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Polygon3D.h"
#include "VICUS_Polygon2D.h"

namespace VICUS {

/*! Class PlaneGeometry encapsulates the vertex data and plane type of a single plane
	in the geometrical model. It also includes subsurfaces and handles triangulation of
	the outer polygon (optionally with holes) and the triangulation of the subsurfaces.

	Also, it implements intersection tests (for picking).

	Usage:

	Set output polygon with setPolygon()
	Set subsurfaces/holes with setHoles().

	This will always update the internal triangulation.
*/
class PlaneGeometry {
public:

	/*! Simple storage member to hold vertex indexes of a single triangle.
		\sa triangles()
	*/
	struct triangle_t {
		triangle_t() {}
		triangle_t(unsigned short i1, unsigned short i2, unsigned short i3) :
			a(i1), b(i2), c(i3)
		{}
		unsigned short a,b,c;
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Default constructor. */
	PlaneGeometry() {}

	/*! Initializing constructor.
		Vertexes a, b and c must be given in counter-clockwise order, so that (b-a) x (c-a) yields the normal vector of the plane.
	*/
	PlaneGeometry(Polygon3D::type_t t, const IBKMK::Vector3D & a, const IBKMK::Vector3D & b, const IBKMK::Vector3D & c);

	/*! A polygon is considered "fully valid" for painting and additing to the data structure, if
		it has enough vertexes and can be correctly triangulated (triangles not empty).
	*/
	bool isValid() const { return m_polygon.isValid() && !m_triangles.empty(); }

	/*! Return the inclination in Deg. 0° -> Roof; 90° -> Wall; 180° -> Floor. */
	double inclination() const;

	/*! Return the orientation in Deg. 0° -> North; 90° -> East; 180° -> South; etc. */
	double orientation() const;

	const IBKMK::Vector3D & normal() const { return m_polygon.normal(); }
	const IBKMK::Vector3D & localX() const { return m_polygon.localX(); }
	const IBKMK::Vector3D & localY() const { return m_polygon.localY(); }
	/*! Returns the offset point (origin of the plane's local coordinate system) */
	const IBKMK::Vector3D & offset() const { return m_polygon.vertexes()[0]; }

	/*! Adds a new 2D vertex in the plane of the outer polygon.
		Calculates 3D vertex coordinates.
	*/
	void addVertex(const IBKMK::Vector2D & v);

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
	*/
	bool intersectsLine(const IBKMK::Vector3D & p1,
						const IBKMK::Vector3D & d,
						IBKMK::Vector3D & intersectionPoint,
						double & dist,
						bool hitBackfacingPlanes = false,
						bool endlessPlane = false) const;

	/*! Returns current vector of triangles of the opaque surface (not including holes). */
	const std::vector<triangle_t> & triangles() const { return m_triangles; }

	/*! Returns the vertexes used by the triangles.
		This is a combination of the polygon's vertexes and vertexes of any (valid) holes inside
		the polygon.
		Note: you need to transfer these polygons to the graphics pipeline.

		Note: The first vertex is always the first vertex in the polygon.
	*/
	const std::vector<IBKMK::Vector3D> & triangleVertexes() const { return m_triangleVertexes; }

	/*! Returns the stored polygon. */
	const Polygon3D & polygon() const { return m_polygon; }
	/*! Sets a new polygon and updates triangulation. */
	void setPolygon(const Polygon3D & polygon3D);

	/*! Returns the vector of holes (2D polygons in the plane of the polygon). */
	std::vector<Polygon2D> holes() const;
	/*! Sets the vector of holes (2D polygons in the plane of the polygon). */
	void setHoles(const std::vector<Polygon2D> & holes);

	/*! Returns the 2D polygon (only if it exists) in the plane of the polygon. */
	const Polygon2D & polygon2D() const { return m_polygon2D; }

	/*! Calculates surface area in m2. */
	double area() const;

	/*! Calculates the center point of the surface/polygon */
	IBKMK::Vector3D centerPoint() const;


private:
	// *** PRIVATE MEMBER FUNCTIONS ***

	/*! This computes the normal vector, performs the triangulation and attempts to simplify a polygon to a rectangle/triangle
		primitive.
		This function is called from all functions that modify the internal polygon/holes vertexes.
	*/
	void computeGeometry();

	/*! Creates a 2D representation of the 3D polygon.
		Function updateLocalCoordinateSystem() must compute first.
		\sa updateLocalCoordinateSystem()
	*/
	bool update2DPolygon();

	/*! This function triangulates the geometry and populate the m_triangles vector.
		This function is called from computeGeometry().
	*/
	void triangulate();


	// *** PRIVATE MEMBER VARIABLES ***

	/*! Contains the information about the polygon that encloses this surface. */
	Polygon3D							m_polygon;

	/*! Polyline in 2D-coordinates in the plane of the polygon. */
	Polygon2D							m_polygon2D;

	/*! Polygons with holes/subsurfaces inside the polygon. */
	std::vector<Polygon2D>				m_holes;

	/*! Contains the vertex indexes for each triangle that the polygon is composed of.
		Includes only the triangles of the opaque surfaces without any holes
	*/
	std::vector<triangle_t>				m_triangles;

	/*! The vertexes used by the triangles. */
	std::vector<IBKMK::Vector3D>		m_triangleVertexes;
};

} // namespace VICUS

#endif // VICUS_PlaneGeometryH
