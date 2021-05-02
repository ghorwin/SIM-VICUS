#ifndef VICUS_Polygon3DH
#define VICUS_Polygon3DH

#include <vector>
#include <IBKMK_Vector3D.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Polygon2D.h"

namespace VICUS {

/*! Class Polygon3D stores a polygon of 3D points that lies in a plane.
	Also provides utility functions for checking and simplifying polygon. The data structure ensures that the
	polygon itselfs is always consistent. If isValid() returns true, it is guarantied to be in a plane, non-winding and without
	consecutive colinear or identical points. Therefore, the polygon can be triangulated right away.
*/
class Polygon3D {
public:

	/*! Different types of the plane described by the polygon. */
	enum type_t {
		/*! Triangle defined through three vertices a, b, c. Triangulation is trivial. */
		T_Triangle,
		/*! Rectangle/Parallelogram defined through four vertices (a, b, c and d), where c = a + (b-a) + (d-a).
			Triangulation gives two triangles.
		*/
		T_Rectangle,
		/*! Polygon, generic polygon with n points. */
		T_Polygon,
		NUM_T
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	Polygon3D() = default;
	Polygon3D(const std::vector<IBKMK::Vector3D> & vertexes);

	void readXML(const TiXmlElement * element);
	TiXmlElement * writeXML(TiXmlElement * parent) const;
	VICUS_COMP(Polygon3D)

	/*! Returns the type of the polygon (can be used to optimize some algorithms). */
	type_t type() const { return m_type; }

	/*! A polygon is considered "fully valid" for painting and additing to the data structure, if
		it has enough vertexes and can be correctly triangulated (triangles not empty).
	*/
	bool isValid() const { return m_valid; }

	/*! Adds a new 2D vertex in the plane of the polygon. Afterwards simplifies polygon. */
	void addVertex(const IBK::point2D<double> & v);

	/*! Removes the vertex at given location.
		\warning Throws an exception if index is out of range.
	*/
	void removeVertex(unsigned int idx);

	/*! Inverts vertexes so that normal vector is inverted/flipped. */
	void flip();

	/*! Returns 3D vertex coordinates. */
	std::vector<IBKMK::Vector3D> vertexes() const { return m_vertexes; }

	/*! Sets all vertexes. */
	void setVertexes(const std::vector<IBKMK::Vector3D> & vertexes);

	/*! Returns the normal vector of the polygon (only defined if polygon is valid).
		Normal vector is defined based on winding order of polygon.
	*/
	const IBKMK::Vector3D & normal() const { return m_normal; }

	const IBKMK::Vector3D & localX() const { return m_localX; }
	const IBKMK::Vector3D & localY() const { return m_localY; }


private:

	// *** PRIVATE MEMBER FUNCTIONS ***

	/*! This function checks the polygon for validity. This function is called automatically from readXML() and
		from addVertex() and removeVertex().
	*/
	void checkPolygon();

	/*! Detects if a polygon geometry with 4 vertices is actually a Rectangle (if the polygon has exactly 4 vertexes and
		vertex #3 can be constructed from adding (v2-v1) and (v4-v1) to v1 with some small rounding error tolerance).
		Polyons with 3 vertexes are Triangles. All others are generic polygons.
	*/
	void detectType();

	/*! A simple polygon is a polygon without intersects by itself.
		return true if no intersections
		return false if minimum one intersection
	*/
	bool isSimplePolygon();

	/*! Eleminate colinear points in a polygon and return a new polygon. */
	void eleminateColinearPts();

	/*! Computes the normal vector of the plane and caches it in m_normal.
		If calculation is not possible (collinear vectors, vectors have zero lengths etc.), the
		normal vector is set to 0,0,0).
	*/
	void updateLocalCoordinateSystem();

	// *** PRIVATE MEMBER VARIABLES ***

	/*! Stores the vertexes in 3D of the polygon. */
	std::vector<IBKMK::Vector3D>		m_vertexes;

	/*! Type of the plane.
		T_POLYGON is the most generic, yet T_TRIANGLE and T_RECTANGLE offer some specialized handling for
		intersection calculation and triangulation.
	*/
	type_t								m_type = NUM_T;

	/*! Stores the valid state of the polygon, update in checkPolygon() */
	bool								m_valid = false;

	/*! Polyline in 2D-coordinates. */
	Polygon2D							m_polygon;

	/*! Normal vector of plane, updated in updateLocalCoordinateSystem(). */
	IBKMK::Vector3D						m_normal = IBKMK::Vector3D(0,0,0);

	/*! Local X-vector, updated in updateLocalCoordinateSystem(). */
	IBKMK::Vector3D						m_localX;
	/*! Local Y-vector, updated in updateLocalCoordinateSystem(). */
	IBKMK::Vector3D						m_localY;

};

} // namespace VICUS

#endif // VICUS_Polygon3DH
