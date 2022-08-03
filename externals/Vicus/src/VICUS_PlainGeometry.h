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

#ifndef VICUS_PlainGeometryH
#define VICUS_PlainGeometryH

#include <IBKMK_Vector3D.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Surface.h"
#include "VICUS_Polygon2D.h"
#include "VICUS_PlaneTriangulationData.h"

namespace VICUS {

/*! Class PlainGeometry encapsulates all plain geometry objects and holds information,
	whether all elements are selected and/or visible.
*/
class PlainGeometry {
	VICUS_READWRITE_PRIVATE
public:

	VICUS_READWRITE

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Default constructor. */
	PlainGeometry() {}

	/*! Polygons with holes/subsurfaces inside the polygon. */
	std::vector<Surface>						m_surfaces; 			// XML:E

	/*! Indicates whether all children elements are visible. */
	bool										m_visible;				// XML:A

	/*! Indicates whether all children elements are selected. */
	bool										m_selected;				// XML:A


private:

	// *** PRIVATE MEMBER FUNCTIONS ***


	// *** PRIVATE MEMBER VARIABLES ***


};

} // namespace VICUS

#endif // VICUS_PlainGeometryH
