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

#include "IBKMK_3DCalculations.h"

namespace VICUS {

void Room::calculateFloorArea() {
	double floorarea = 0;
	for(const VICUS::Surface & s : m_surfaces){
		Polygon3D poly = s.polygon3D();
		// include all surfaces which normal a quiet similar to -Z normal (5 DEG deviation)
		double cosAlpha = std::cos(5*IBK::DEG2RAD);
		double scalarProduct = poly.normal().scalarProduct(IBKMK::Vector3D(0,0,-1));
		if(scalarProduct >= cosAlpha )
			floorarea += s.geometry().area(2);
	}

	m_para[P_Area].set(floorarea, IBK::Unit("m2"));
}

void Room::calculateVolume() {
	double vol = 0;
	for(const VICUS::Surface & s : m_surfaces){

		std::vector<IBKMK::Vector3D> vertices = s.geometry().triangulationData().m_vertexes;

		if(vertices.size() != 3)
			continue;

		const IBKMK::Vector3D & p0 = vertices[0];
		const IBKMK::Vector3D & p1 = vertices[1];
		const IBKMK::Vector3D & p2 = vertices[2];

		vol += (( p1.m_y - p0.m_y ) * ( p2.m_z - p0.m_z ) - ( p1.m_z - p0.m_z ) * ( p2.m_y - p0.m_y ))
				* ( p0.m_x + p1.m_x + p2.m_x );
	}

	vol /= 6;
	m_para[P_Area].set(vol, IBK::Unit("m3"));

}


} // namespace VICUS
