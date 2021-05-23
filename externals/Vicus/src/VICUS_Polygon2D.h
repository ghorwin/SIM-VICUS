/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  ... all the others from the SIM-VICUS team ... :-)

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#ifndef VICUS_Polygon2DH
#define VICUS_Polygon2DH

#include <vector>
#include <IBKMK_Vector2D.h>

#include "VICUS_CodeGenMacros.h"

namespace VICUS {

/*! Class Polygon2D stores a polygon of 2D points.
	Also provides utility functions for checking and simplifying polygon. The data structure ensures that the
	polygon itselfs is always consistent, i.e. if isValid() returns true, it is guarantied to be non-winding and without
	consecutive colinear or identical points. Therefore, the polygon can be triangulated right away.
*/
class Polygon2D {
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

	Polygon2D() = default;
	Polygon2D(const std::vector<IBKMK::Vector2D> & vertexes);

	void readXML(const TiXmlElement * element);
	TiXmlElement * writeXML(TiXmlElement * parent) const;
	VICUS_COMP(Polygon2D)

	/*! Returns the type of the polygon (can be used to optimize some algorithms). */
	type_t type() const { return m_type; }

	/*! A polygon is considered "fully valid" for painting and additing to the data structure, if
		it has enough vertexes and can be correctly triangulated (triangles not empty).
	*/
	bool isValid() const { return m_valid; }

	/*! Resets the polygon. */
	void clear() { m_type = NUM_T; m_vertexes.clear(); m_valid = false; }

	/*! Adds a new 2D vertex in the plane of the polygon. Afterwards simplifies polygon. */
	void addVertex(const IBK::point2D<double> & v);

	/*! Removes the vertex at given location.
		\warning Throws an exception if index is out of range.
	*/
	void removeVertex(unsigned int idx);

	/*! Inverts vertexes so that normal vector is inverted/flipped. */
	void flip();

	/*! Check for intersection of each edge with the line(p1, p2). Returns true if intersection was found and in this
		case stores the computed intersection point.
	*/
	bool intersectsLine2D(const IBK::point2D<double> &p1, const IBK::point2D<double> &p2, IBK::point2D<double> & intersectionPoint) const;

	/*! Returns 2D coordinates of the polygon. */
	const std::vector<IBKMK::Vector2D> & vertexes() const { return m_vertexes; }

	void setVertexes(const std::vector<IBKMK::Vector2D> & vertexes);

	/*! Calculates surface area in m2. */
	double area() const;

	/*! A simple polygon is a polygon without intersects by itself.
		return true if no intersections
		return false if minimum one intersection
	*/
	bool isSimplePolygon() const;


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

	/*! Eleminate colinear points in a polygon and return a new polygon. */
	void eleminateColinearPts();

	// *** PRIVATE MEMBER VARIABLES ***

	/*! Type of the plane.
		T_POLYGON is the most generic, yet T_TRIANGLE and T_RECTANGLE offer some specialized handling for
		intersection calculation and triangulation.
	*/
	type_t								m_type = NUM_T;

	/*! Points of polyline (in double-precision accuracy!).
		\warning Do not write to this variable, unless you know what you are doing. Rather use addVertex().
	*/
	std::vector<IBKMK::Vector2D>		m_vertexes;

	/*! Stores the valid state of the polygon, update in checkPolygon() */
	bool								m_valid = false;
};

} // namespace VICUS

#endif // VICUS_Polygon2DH
