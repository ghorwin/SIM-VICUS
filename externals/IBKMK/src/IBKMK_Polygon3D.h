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

#ifndef IBKMK_Polygon3DH
#define IBKMK_Polygon3DH

#include "IBKMK_Vector3D.h"
#include "IBKMK_Polygon2D.h"

namespace IBKMK {

/*! Class Polygon3D stores a polygon of 3D points that lies in a plane.
	Also provides utility functions for checking and simplifying polygon. The data structure ensures that the
	polygon itselfs is always consistent. If isValid() returns true, it is guarantied to be in a plane,
	non-winding and without consecutive colinear or identical points. Therefore, the polygon can be
	triangulated right away.

	Internally, the 3D polygon is stored as a 2D Polygon that is placed and oriented in space through
	offset, normal and xAxis vector (the yAxis vector is automatically computed and cached).

	There are several functions for constructing a polygon. All have in common that passing invalid arguments may
	lead to an invalid polygon. Hence, checking the correctness of the polygon after construction is mandatory!

	\note When constructing a polygon with invalid data (polyline, or vectors), the 2D polyline might be empty.
		  You cannot use the polygon to construction incrementally a valid polygon! Always check for validity before
		  using any query functions.
*/
class Polygon3D {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	Polygon3D() = default;

	/*! Initializing constructor.
		Vertexes a, b and c must be given in counter-clockwise order, so that (b-a) x (c-a) yields the normal vector of the plane.
		If t is Polygon2D::T_Rectangle, vertex c actually corresponds to vertex d of the rectangle, and vertex c is computed
		internally.
	*/
	Polygon3D(Polygon2D::type_t t, const IBKMK::Vector3D & a, const IBKMK::Vector3D & b, const IBKMK::Vector3D & c);

	/*! Constructs a polygon from 2D polygon with normal vector, xaxis and offset. */
	Polygon3D(const Polygon2D & p2d, const Vector3D & offset, const IBKMK::Vector3D & normal, const IBKMK::Vector3D & localX);

	/*! Constructs a polygon from a 3D polyline (which might be invalid in any number of ways).
		The normal vector will be deduced from rotation direction of the polygon, and the x-axis vector will be the vector
		from first to second vertex at a suitable (automatically selected) vertex of the polygon.

		\note Once all collinear points have been removed the offset point will be the first vertex of the polygon. Use offset()
			to retrieve the offset.
	*/
	Polygon3D(const std::vector<IBKMK::Vector3D> & vertexes);

	/*! This function works pretty much like the constructor taking a vector of vertexes.
		However, when heal=true some fixing is attempted when a vertex is too far out of plane (mostly due to rounding errors).
	*/
	bool setVertexes(const std::vector<IBKMK::Vector3D> & vertexes, bool heal = true);

	/*! Returns true, if both the polyline itself and the x and normal vectors are valid. */
	bool isValid() const { return m_valid; }

	/*! Comparison operator != */
	bool operator!=(const Polygon3D &other) const;

	// Query functions - do not call for invalid polygons, since they may be at best undefined.

	/*! Returns the type of the polygon (can be used to optimize some algorithms).
		\note Result of this function is undefined for invalid polygons.
	*/
	Polygon2D::type_t type() const { return m_polyline.type(); }

	/*! Returns 3D vertex coordinates. */
	const std::vector<IBKMK::Vector3D> & vertexes() const;
	const std::vector<IBKMK::Vector3D> & rawVertexes() const;

	const IBKMK::Vector3D & offset() const { return m_offset; }
	const IBKMK::Vector3D & normal() const { return m_normal; }
	const IBKMK::Vector3D & localX() const { return m_localX; }
	const IBKMK::Vector3D & localY() const { return m_localY; }

	const Polygon2D & polyline() const { return m_polyline; }


	// Transformation functions

	/*! Totates the polygon/plane and updates the y-axis.
		Throws an exception if either normal or xAxis do not have unit length, or if both vectors are colliniar.
	*/
	void setRotation(const IBKMK::Vector3D & normal, const IBKMK::Vector3D & localX);
	/*! Moves the polygon in 3D space by 'distance'. */
	void setTranslation(const IBKMK::Vector3D & offset) { m_offset = offset; m_dirty = true; }
	/*! Moves the polygon in 3D space by 'distance'. */
	void translate(const IBKMK::Vector3D & distance) { m_offset += distance; m_dirty = true; }
	/*! Inverts normal vector (this is a convenience function for setting a new orientation
		with negated normal vector.
	*/
	void flip();


	// Calculation functions

	/*! Returns the center point (average of all vertexes of the polygon).
		\note Throws an exception if polygon is invalid.
	*/
	IBKMK::Vector3D centerPoint() const;

	/*! Computes bounding box of polygon.
		An invalid/empty polygon does not have bounding box and an exception will be thrown
		(check with isValid() beforehand).
	*/
	void boundingBox(IBKMK::Vector3D & lowerValues, IBKMK::Vector3D & upperValues) const;

	/*! Enlarges existing bounding box to hold polygon.
		An invalid/empty polygon does not have bounding box and an exception will be thrown
		(check with isValid() beforehand).
	*/
	void enlargeBoundingBox(IBKMK::Vector3D & lowerValues, IBKMK::Vector3D & upperValues) const;



private:

	// *** PRIVATE MEMBER FUNCTIONS ***

	/*! Computes the normal vector of the plane and caches it in m_normal.
		If calculation is not possible (collinear vectors, vectors have zero lengths etc.), the
		normal vector is set to 0,0,0).
	*/
	void updateLocalCoordinateSystem(const std::vector<IBKMK::Vector3D> & verts);

	/*! Computes the 2D polyline (polygon's vertex coordinates projected onto the
		xy-plane of the polygon's local coordinate system).
		Requires valid normal, localX, and localY vectors to be stored in members already
	*/
	void update2DPolyline(const std::vector<IBKMK::Vector3D> & verts);

	/*! Checks if the vector is smaller zero. */
	bool smallerVectZero(const IBKMK::Vector3D &vect);

	/*! Calculates a normalized normal of a polyline. If an error occurs the vector (1,0,0) is output.  */
	IBKMK::Vector3D computeNormal(const std::vector<IBKMK::Vector3D>& polygon);

	// *** PRIVATE MEMBER VARIABLES ***

	/*! Stores the valid state of the polygon, update in checkPolygon() */
	bool									m_valid = false;

	/*! Offset of the polygon. */
	IBKMK::Vector3D							m_offset;
	/*! Normal vector. */
	IBKMK::Vector3D							m_normal;
	/*! x-Axis-vector. */
	IBKMK::Vector3D							m_localX;
	/*! y-Axis-vector (computed from x and normal, not stored in data model). */
	IBKMK::Vector3D							m_localY;
	/*! The 2D polyline. */
	Polygon2D								m_polyline;

	/*! Dirty flag, set to true whenever anything is modified that affects the 3D vertex coordinates. */
	mutable	bool							m_dirty;
	/*! Cached 3D vertexes, updated upon access when dirty is true. */
	mutable	std::vector<IBKMK::Vector3D>	m_vertexes;


};

} // namespace IBKMK

#endif // IBKMK_Polygon3DH
