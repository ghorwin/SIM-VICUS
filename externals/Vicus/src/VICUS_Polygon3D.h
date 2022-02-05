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

#ifndef VICUS_Polygon3DH
#define VICUS_Polygon3DH

#include <IBKMK_Vector3D.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Polygon2D.h"

namespace VICUS {

/*! Class Polygon3D stores a polygon of 3D points that lies in a plane.
	The polygon is defined by means of a 2D polyline that is projected onto a plane in space.
	This plane is defined through m_offset, m_normal and m_xAxis vectors.
	\note The polygon implements 2D to 3D coordinate calculation using lazy evaluation. Hence, any operation
		  requiring an update of the 3D vertexes marks the vector as "dirty".
*/
class Polygon3D {
	VICUS_READWRITE_PRIVATE
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	Polygon3D() = default;

	VICUS_READWRITE
	VICUS_COMP(Polygon3D)

	/*! Returns true, if both the polyline itself and the x and normal vectors are valid. */
	bool isValid() const;

	/*! Totates the polygon/plane and updates the y-axis.
		Throws an exception if either normal or xAxis do not have unit length, or if both vectors are colliniar.
	*/
	void setRotation(const IBKMK::Vector3D & normal, const IBKMK::Vector3D & localX);
	/*! Moves the polygon in 3D space by 'distance'. */
	void setTranslation(const IBKMK::Vector3D & offset) { m_offset = offset; m_dirty = true; }
	/*! Moves the polygon in 3D space by 'distance'. */
	void translate(const IBKMK::Vector3D & distance) { m_offset += distance; m_dirty = true; }
	/*! Returns the center point (average of all vertexes of the polygon). */
	IBKMK::Vector3D centerPoint() const;

	const std::vector<IBKMK::Vector3D> & vertexes() const;

	const IBKMK::Vector3D & normal() const { return m_normal; }
	const IBKMK::Vector3D & localX() const { return m_localX; }
	const IBKMK::Vector3D & localY() const { return m_localY; }

	const Polygon2D & polyline() const { return m_polyline; }

private:
	/*! Offset of the polygon. */
	IBKMK::Vector3D							m_offset;		// XML:E
	/*! Normal vector. */
	IBKMK::Vector3D							m_normal;		// XML:E
	/*! x-Axis-vector. */
	IBKMK::Vector3D							m_localX;		// XML:E
	/*! y-Axis-vector (computed from x and normal, not stored in data model). */
	IBKMK::Vector3D							m_localY;
	/*! The 2D polyline. */
	Polygon2D								m_polyline;		// XML:E

	/*! Dirty flag, set to true whenever anything is modified that affects the 3D vertex coordinates. */
	mutable	bool							m_dirty;
	/*! Cached 3D vertexes, updated upon access when dirty is true set. */
	mutable	std::vector<IBKMK::Vector3D>	m_vertexes;
};

} // namespace VICUS

#endif // VICUS_Polygon3DH
