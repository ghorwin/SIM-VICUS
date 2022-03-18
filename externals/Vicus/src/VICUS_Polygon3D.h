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

#include <IBKMK_Polygon3D.h>

#include "VICUS_CodeGenMacros.h"

namespace VICUS {

/*! Class Polygon3D stores a planar polygon in 3D space.
	This class merely wraps IBKMK::Polygon3D and provides read/write functionality.
*/
class Polygon3D : public IBKMK::Polygon3D {
	VICUS_READWRITE_PRIVATE
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	Polygon3D() = default;

	/*! Conversion constructor */
	explicit Polygon3D(const IBKMK::Polygon3D & poly3D) :
		IBKMK::Polygon3D(poly3D)
	{
	}

	/*! Initializing constructor.
		Vertexes a, b and c must be given in counter-clockwise order, so that (b-a) x (c-a) yields the normal vector of the plane.
		If t is Polygon2D::T_Rectangle, vertex c actually corresponds to vertex d of the rectangle, and vertex c is computed
		internally.
	*/
	Polygon3D(IBKMK::Polygon2D::type_t t, const IBKMK::Vector3D & a, const IBKMK::Vector3D & b, const IBKMK::Vector3D & c) :
		IBKMK::Polygon3D(t, a, b, c)
	{}

	/*! Constructs a polygon from 2D polygon with normal vector, xaxis and offset. */
	Polygon3D(const IBKMK::Polygon2D & p2d, const IBKMK::Vector3D & offset,
			  const IBKMK::Vector3D & normal, const IBKMK::Vector3D & xAxis) :
		IBKMK::Polygon3D(p2d, offset, normal, xAxis)
	{}

	/*! Constructs a polygon from a 3D polyline (which might be invalid in any number of ways).
		The normal vector will be deduced from rotation direction of the polygon, and the x-axis vector will be the vector
		from first to second vertex at a suitable (automatically selected) vertex of the polygon.

		\note Once all collinear points have been removed the offset point will be the first vertex of the polygon. Use offset()
			to retrieve the offset.
	*/
	Polygon3D(const std::vector<IBKMK::Vector3D> & vertexes) :
		IBKMK::Polygon3D(vertexes)
	{}

	void readXML(const TiXmlElement * element);
	TiXmlElement * writeXML(TiXmlElement * parent) const;

	VICUS_COMP(Polygon3D)
};

} // namespace VICUS

#endif // VICUS_Polygon3DH
