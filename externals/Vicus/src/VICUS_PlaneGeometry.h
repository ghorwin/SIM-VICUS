#ifndef VICUS_PlaneGeometryH
#define VICUS_PlaneGeometryH

#include <IBKMK_Vector3D.h>

#include <QVector3D>
#include <QPolygonF>

#include "VICUS_CodeGenMacros.h"


namespace VICUS {

/*! Class PlaneGeometry encapsulates the vertex data and plane type of a single plane
	in the geometrical model.

	Also, it implements intersection tests (for picking).
*/
class PlaneGeometry {
public:

	/*! Different types of the plane. */
	enum type_t {
		/*! Triangle defined through three vertices a, b, c. Triangulation is trivial. */
		T_Triangle,			// Keyword: Triangle
		/*! Rectangle/Parallelogram defined through four vertices (a, b, c and d), where c = a + (b-a) + (d-a).
			Triangulation gives two triangles.
		*/
		T_Rectangle,		// Keyword: Rectangle
		/*! Polygon, generic polygon with n points. */
		T_Polygon,			// Keyword: Polygon
		NUM_T
	};

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
	/*! Default constructor. */
	explicit PlaneGeometry(type_t t) : m_type(t) {}

	/*! Initializing constructor.
		Vertexes a, b and c must be given in counter-clockwise order, so that (b-a) x (c-a) yields the normal vector of the plane.
	*/
	PlaneGeometry(type_t t, const IBKMK::Vector3D & a, const IBKMK::Vector3D & b, const IBKMK::Vector3D & c);

	void readXML(const TiXmlElement * element);
	TiXmlElement * writeXML(TiXmlElement * parent) const;
	VICUS_COMP(PlaneGeometry)

	type_t type() const { return m_type; }

	/*! A polygon is considered "fully valid" for painting and additing to the data structure, if
		it has enough vertexes and can be correctly triangulated (triangles not empty).
	*/
	bool isValid() const { return m_vertexes.size() >= 3 && !m_triangles.empty(); }

	const IBKMK::Vector3D & normal() const { return m_normal; }

	/*! Adds a new 2D vertex in the plane of the polygon.
		Calculates 3D vertex coordinates.
	*/
	void addVertex(const QPointF & v);

	/*! Adds a new 3D vertex.
		Calculates 2D plane coordinates and throws an exception, if vertex is out of plane.
	*/
	void addVertex(const IBKMK::Vector3D & v);

	/*! Removes the vertex at given location. */
	void removeVertex(unsigned int idx);

	/*! This computes the normal vector, performs the triangulation and attempts to simplify a polygon to a rectangle/triangle
		primitive.
		This function is called automatically from readXML().
	*/
	void computeGeometry();

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

	/*! Returns current vector of triangles. */
	const std::vector<triangle_t> & triangles() const { return m_triangles; }

	/*! Returns 3D vertex coordinates. */
	std::vector<IBKMK::Vector3D> vertexes() const {	return m_vertexes; }

	/*! Calculates surface area in m2. */
	double area() const;

	/*! Calculates the center point of the surface */
	IBKMK::Vector3D centerPoint() const;

	void setVertexes(const std::vector<IBKMK::Vector3D> & vertexes);

	/*! Returns the 2D polygon (only if it exists). */
	const QPolygonF & polygon() const { return m_polygon; }

	/*! Returns the x-vector of the local coordinate system. */
	const IBKMK::Vector3D & localX() const { return m_localX; }

	/*! Returns the y-vector of the local coordinate system. */
	const IBKMK::Vector3D & localY() const { return m_localY; }

private:
	// *** PRIVATE MEMBER FUNCTIONS ***

	/*! Attempts to convert a polygon geometry to Rectangle type, if the polygon has exactly 4 vertexes and
		vertex #3 can be constructed from adding (v2-v1) and (v4-v1) to v1 (with some small rounding error tolerance).
		If the polygon cannot be converted, nothing happens.
	*/
	void simplify();

	/*! Creates a 2D representation of the 3D polygon.
		Function updateLocalCoordinateSystem() must compute first.
		\sa updateLocalCoordinateSystem()
	*/
	bool update2DPolygon();

	/*! Creates a 3D representation of the 2D polygon.
		Outdated
	*/
	void update3DPolygon();

	/*! This function triangulates the geometry and populate the m_triangles vector.
		This function is called from updateGeometry().
	*/
	void triangulate();

	/*! Computes the normal vector of the plane and caches it in m_normal.
		If calculation is not possible (collinear vectors, vectors have zero lengths etc.), the
		normal vector is set to 0,0,0).
	*/
	void updateLocalCoordinateSystem();

	/*! A simple polygon is a polygon without intersects by itself.
		return true if no intersections
		return false if minimum one intersection
	*/
	bool isSimplePolygon();

	/*! Eleminate colinear points in a polygon and return a new polygon. */
	void eleminateColinearPts();


	// *** PRIVATE MEMBER VARIABLES ***

	/*! Type of the plane.
		T_POLYGON is the most generic, yet T_TRIANGLE and T_RECTANGLE offer some specialized handling for
		intersection calculation and data transfer to the graphics pipeline.
	*/
	type_t								m_type = NUM_T;

	/*! Points of polyline (in double-precision accuracy!).
		\warning Do not write to this variable, unless you know what you are doing. Rather use addVertex().
	*/
	std::vector<IBKMK::Vector3D>		m_vertexes;

	// *** Runtime Variables ***

	/*! Polyline in 2D-coordinates. */
	QPolygonF							m_polygon;

	/*! Normal vector of plane, updated in updateNormal(). */
	IBKMK::Vector3D						m_normal = IBKMK::Vector3D(0,0,0);

	/*! Contains the vertex indexes for each triangle that the polygon is composed of (in anti-clock-wise order, so
		that (b-a) x (c-a) gives the normal vector of the plane.
		This vector is updated in computeGeometry()/triangulate().
	*/
	std::vector<triangle_t>				m_triangles;

	IBKMK::Vector3D						m_localX;
	IBKMK::Vector3D						m_localY;

private:
	void readXMLPrivate(const TiXmlElement * element);
	TiXmlElement * writeXMLPrivate(TiXmlElement * parent) const;
};

} // namespace VICUS

#endif // VICUS_PlaneGeometryH
