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

#ifndef VICUS_Polygon2DH
#define VICUS_Polygon2DH

#include <IBKMK_Polygon2D.h>

#include "VICUS_CodeGenMacros.h"

namespace VICUS {

/*! Class Polygon2D stores a polygon of 2D points.
	This class merely wraps IBKMK::Polygon2D and provides read/write functionality.
*/
class Polygon2D : public IBKMK::Polygon2D {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	Polygon2D() = default;
	Polygon2D(const std::vector<IBKMK::Vector2D> & vertexes) :
		IBKMK::Polygon2D(vertexes)
	{
	}

	void readXML(const TiXmlElement * element);
	TiXmlElement * writeXML(TiXmlElement * parent) const;

	VICUS_COMP(Polygon2D)
};

} // namespace VICUS

#endif // VICUS_Polygon2DH
