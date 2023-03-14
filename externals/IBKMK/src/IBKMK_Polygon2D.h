/*	IBK Math Kernel Library
	Copyright (c) 2001-today, Institut fuer Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, A. Paepcke, H. Fechner, St. Vogelsang
	All rights reserved.

	This file is part of the IBKMK Library.

	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
	   list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation
	   and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors
	   may be used to endorse or promote products derived from this software without
	   specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
	ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	This library contains derivative work based on other open-source libraries,
	see LICENSE and OTHER_LICENSES files.

*/

#ifndef IBKMK_Polygon2DH
#define IBKMK_Polygon2DH

#include <vector>
#include "IBKMK_Vector2D.h"

namespace IBKMK {

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

	/*! Comparison operator. */
	bool operator!=(const Polygon2D &other) const;

	/*! Returns the type of the polygon (can be used to optimize some algorithms). */
	type_t type() const { return m_type; }

	/*! A polygon is considered "fully valid" for painting and additing to the data structure, if
		it has enough vertexes and can be correctly triangulated (triangles not empty).
	*/
	bool isValid() const { return m_valid; }

	/*! Resets the polygon. */
	void clear() { m_type = NUM_T; m_vertexes.clear(); m_valid = false; }

	/*! Check for intersection of each edge with the line(p1, p2). Returns true if intersection was found and in this
		case stores the computed intersection point.
	*/
	bool intersectsLine2D(const IBK::point2D<double> &p1, const IBK::point2D<double> &p2, IBK::point2D<double> & intersectionPoint) const;

	/*! Returns 2D coordinates of the polygon. */
	const std::vector<IBKMK::Vector2D> & vertexes() const { return m_vertexes; }

	/*! Sets vertexes of the polygon.
		After the vertexes have been transfered, the polygon is checked, and potentially duplicate vertexes are removed.
		Finally, the type of the polygon is detected and if all is well, the polygon is marked as valid.

		\note Do not rely on the list of vertexes passed to the polygon to be returned unmodified!
	*/
	void setVertexes(const std::vector<IBKMK::Vector2D> & vertexes);

	/*! Computes the bounding box from the vertexes.
		Requires at least one vertex in the polygon.
	*/
	void boundingBox(IBKMK::Vector2D & lowerValues, IBKMK::Vector2D & upperValues) const;

	/*! Calculates surface area in m2.
		\note Throws an exception for invalid polygons, otherwise computes the area.
	*/
	double area(int digits = 1) const;
	/*! Calculates surface area in m2 but the result is signed. */
	double areaSigned(int digits) const;

	/*! Calculates surface circumference in m.
		\note Throws an exception for invalid polygons, otherwise computes the circumference.
	*/
	double circumference() const;

	/*! A simple polygon is a polygon without intersects by itself.
		Returns true if no intersections.
		Return false if at least one intersection was found.
	*/
	bool isSimplePolygon() const;


protected:

	// *** PRIVATE MEMBER FUNCTIONS ***

	/*! This function checks the polygon for validity. This function is called automatically from readXML() and
		from addVertex() and removeVertex().
		\note Does not throw an exception.
	*/
	void checkPolygon();

	/*! Detects if a polygon geometry with 4 vertices is actually a Rectangle (if the polygon has exactly 4 vertexes and
		vertex #3 can be constructed from adding (v2-v1) and (v4-v1) to v1 with some small rounding error tolerance).
		Polyons with 3 vertexes are Triangles. All others are generic polygons.
		\note Does not throw an exception.
	*/
	void detectType();

	/*! Eleminate colinear points in a polygon and return a new polygon.
		\note Does not throw an exception.
	*/
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

} // namespace IBKMK

#endif // IBKMK_Polygon2DH
