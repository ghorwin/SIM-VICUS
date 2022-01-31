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
	double floorarea = 0;
	for(const VICUS::Surface & s : m_surfaces){
		Polygon3D poly = s.polygon3D();
		// include all surfaces which normal a quiet similar to -Z normal (5 DEG deviation)
		double cosAlpha = std::cos(5*IBK::DEG2RAD);
		double scalarProduct = poly.normal().scalarProduct(IBKMK::Vector3D(0,0,-1));
		if(scalarProduct >= cosAlpha )
			floorarea += s.geometry().area(2);
	}
	VICUS::KeywordList::setParameter(m_para,"Room::para_t", VICUS::Room::P_Area, floorarea);
}

void Room::calculateVolume() {
	double vol = 0;
	for(const VICUS::Surface & s : m_surfaces){

		PlaneGeometry pg;
		// in s.geometry() are only these polygons which are clipped by holes ...
		// we need all opake surfaces without hole
		// so triangulate again
		pg.setPolygon(s.polygon3D());

		const PlaneTriangulationData planeTri = pg.triangulationData();
//		std::cout << "Surface Name:\t" << s.m_displayName.toStdString() << std::endl;

		// Opaque surfaces (these may contain holes such as windows)
		for(unsigned int i=0; i<planeTri.m_triangles.size(); ++i){
			const IBKMK::Triangulation::triangle_t &tri = planeTri.m_triangles[i];
			if(tri.isDegenerated())
				continue;
			const IBKMK::Vector3D & p0 = planeTri.m_vertexes[tri.i1];
			const IBKMK::Vector3D & p1 = planeTri.m_vertexes[tri.i2];
			const IBKMK::Vector3D & p2 = planeTri.m_vertexes[tri.i3];

			vol += (( p1.m_y - p0.m_y ) * ( p2.m_z - p0.m_z ) - ( p1.m_z - p0.m_z ) * ( p2.m_y - p0.m_y ))
					* ( p0.m_x + p1.m_x + p2.m_x );

			// ToDo Anne: Das hier bitte prüfen
//			std::cout << "\tTeil" << i << ":" << std::endl;
//			std::cout << "\tP0:\t" << p0.m_x << "\t" << p0.m_y << "\t" << p0.m_z <<  "\tNormale:\t" << planeTri.m_normal.m_x << "\t" << planeTri.m_normal.m_y << "\t" << planeTri.m_normal.m_z << std::endl;
//			std::cout << "\tP1:\t" << p1.m_x << "\t" << p1.m_y << "\t" << p1.m_z << std::endl;
//			std::cout << "\tP2:\t" << p2.m_x << "\t" << p2.m_y << "\t" << p2.m_z << std::endl;

		}

	}

	vol /= 6; // FIXME: funktioniert bei Dächern nicht!

	VICUS::KeywordList::setParameter(m_para,"Room::para_t", VICUS::Room::P_Volume, vol);


}


} // namespace VICUS
