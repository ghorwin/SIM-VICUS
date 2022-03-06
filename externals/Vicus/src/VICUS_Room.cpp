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

#include "VICUS_Room.h"
#include "VICUS_KeywordList.h"

#include "IBKMK_3DCalculations.h"

namespace VICUS {

void Room::calculateFloorArea() {
	// All surfaces with near vertical normal vector (pointing up) are assumed to be part of the
	// floor area and their polygon areas are summed up.
	const double cosAlpha = std::cos(5*IBK::DEG2RAD);
	double floorarea = 0;
	for (const VICUS::Surface & s : m_surfaces) {
		// include all surfaces which normal a quiet similar to -Z normal (5 DEG deviation)
		double scalarProduct = s.geometry().normal().scalarProduct(IBKMK::Vector3D(0,0,-1));
		if (scalarProduct >= cosAlpha )
			floorarea += s.geometry().area(2);
	}
	VICUS::KeywordList::setParameter(m_para,"Room::para_t", VICUS::Room::P_Area, floorarea);
}


void Room::calculateVolume() {
	// an implementation of Shoelace Formula, see https://ysjournal.com/tetrahedral-shoelace-method-calculating-volume-of-irregular-solids/
	//
	// Conditions:
	//   - requires room to be fully enclosed by surfaces
	//   - all surfaces must have normal vector pointing _into_ the room

	double vol = 0;
	// process all surfaces
	for (const VICUS::Surface & s : m_surfaces) {
		const PlaneTriangulationData & planeTri = s.geometry().triangulationDataWithoutHoles();

		// process all triangles
		for (unsigned int i=0; i<planeTri.m_triangles.size(); ++i) {
			const IBKMK::Triangulation::triangle_t &tri = planeTri.m_triangles[i];
			// Note: plane geometry takes care not to add degenerated triangles to triangulation data,
			//       but we add an ASSERT to make sure
			IBK_ASSERT(!tri.isDegenerated());

			// now compute determinant of matrix from points p0, p1, p2 and points [0,0,0]
			const IBKMK::Vector3D & p0 = planeTri.m_vertexes[tri.i1];
			const IBKMK::Vector3D & p1 = planeTri.m_vertexes[tri.i2];
			const IBKMK::Vector3D & p2 = planeTri.m_vertexes[tri.i3];

			vol +=	p0.m_x * p1.m_y * p2.m_z +
					p2.m_x * p0.m_y * p1.m_z +
					p1.m_x * p2.m_y * p0.m_z

					- p2.m_x * p1.m_y * p0.m_z
					- p0.m_x * p2.m_y * p1.m_z
					- p1.m_x * p0.m_y * p2.m_z;
		}
	}

	vol /= 6;

	VICUS::KeywordList::setParameter(m_para,"Room::para_t", VICUS::Room::P_Volume, vol);
}


} // namespace VICUS
