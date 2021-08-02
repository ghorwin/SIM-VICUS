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

/*! Class Polygon3D stores a polygon of 3D points that lies in a plane.
	Also provides utility functions for checking and simplifying polygon. The data structure ensures that the
	polygon itselfs is always consistent. If isValid() returns true, it is guarantied to be in a plane, non-winding and without
	consecutive colinear or identical points. Therefore, the polygon can be triangulated right away.
*/
class Polygon3D : public IBKMK::Polygon3D {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	Polygon3D() = default;
	Polygon3D(const std::vector<IBKMK::Vector3D> & vertexes) :
		IBKMK::Polygon3D(vertexes)
	{
	}

	/*! Initializing constructor.
		Vertexes a, b and c must be given in counter-clockwise order, so that (b-a) x (c-a) yields the normal vector of the plane.
	*/
	Polygon3D(IBKMK::Polygon3D::type_t t, const IBKMK::Vector3D & a, const IBKMK::Vector3D & b, const IBKMK::Vector3D & c) :
		IBKMK::Polygon3D(t, a, b, c)
	{
	}

	void readXML(const TiXmlElement * element);
	TiXmlElement * writeXML(TiXmlElement * parent) const;

	VICUS_COMP(Polygon3D)
};

} // namespace VICUS

#endif // VICUS_Polygon3DH
