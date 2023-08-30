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

#include <QDebug>

#include <IBKMK_3DCalculations.h>
#include <IBK_messages.h>

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
	FUNCID(Room::calculateVolume);

	// an implementation of Shoelace Formula, see https://ysjournal.com/tetrahedral-shoelace-method-calculating-volume-of-irregular-solids/
	//
	// Conditions:
	//   - requires room to be fully enclosed by surfaces
	//   - all surfaces must have normal vector pointing _into_ the room

//	qDebug() << "Volumenberechnung";

	double vol = 0;
	unsigned int surfaceCounter = 0;
	// process all surfaces
	for (const VICUS::Surface & s : m_surfaces) {

		const PlaneTriangulationData &planeTri1 = s.geometry().triangulationData();
		const std::vector<PlaneTriangulationData> &holes = s.geometry().holeTriangulationData();

//		qDebug() << "Fläche: " << s.m_displayName;
//		qDebug() << "Flächennormale"
//				 << "\t" << s.geometry().normal().m_x
//				 << "\t" << s.geometry().normal().m_y
//				 << "\t" << s.geometry().normal().m_z;

		// process all triangles of all holes
		unsigned int holeCounter=0;
//		if(!holes.empty()){
//			qDebug() << "Fläche enthält " << holes.size() << " Löcher";
//			qDebug() << "Lochnr." << "\t" << "Teil" << "\t" << "Vol.berechnung";
//		}
		for(const PlaneTriangulationData &hole : holes){

			for (unsigned int i=0; i<hole.m_triangles.size(); ++i){
				const IBKMK::Triangulation::triangle_t &tri = hole.m_triangles[i];
				// Note: plane geometry takes care not to add degenerated triangles to triangulation data,
				//       but we add an ASSERT to make sure
				IBK_ASSERT(!tri.isDegenerated());

				// now compute determinant of matrix from points p0, p1, p2 and points [0,0,0]
				IBKMK::Vector3D p0 = hole.m_vertexes[tri.i1];
				IBKMK::Vector3D p1 = hole.m_vertexes[tri.i2];
				IBKMK::Vector3D p2 = hole.m_vertexes[tri.i3];

				IBKMK::Vector3D nTri = (p1-p0).crossProduct(p2-p0);
				nTri.normalize();
				IBKMK::Vector3D nDiff = nTri - s.geometry().normal();
				// double x = nDiff.magnitude();
				if(nDiff.magnitude() > 1.5 ){
					// jetzt stehen die Normalen in unterschiedliche Richtungen
					// erwartet wird, dass die Normalen in die gleiche Richtung zeigen
					// Daher wird die Reihenfolge der Punkte vertauscht

					p2 = planeTri1.m_vertexes[tri.i2];
					p1 = planeTri1.m_vertexes[tri.i3];

				}

				vol +=	p0.m_x * p1.m_y * p2.m_z +
						p2.m_x * p0.m_y * p1.m_z +
						p1.m_x * p2.m_y * p0.m_z

						- p2.m_x * p1.m_y * p0.m_z
						- p0.m_x * p2.m_y * p1.m_z
						- p1.m_x * p0.m_y * p2.m_z;

//				qDebug() << holeCounter << "\t" << i << "\t" << volDebug;
				++holeCounter;
			}
		}

		// process all triangles outer bound

//		qDebug() << "Flächennr.\tTeil\tVol.berechnung";
		for (unsigned int i=0; i<planeTri1.m_triangles.size(); ++i) {
			const IBKMK::Triangulation::triangle_t &tri = planeTri1.m_triangles[i];
			// Note: plane geometry takes care not to add degenerated triangles to triangulation data,
			//       but we add an ASSERT to make sure
			IBK_ASSERT(!tri.isDegenerated());

			// now compute determinant of matrix from points p0, p1, p2 and points [0,0,0]
			IBKMK::Vector3D p0 = planeTri1.m_vertexes[tri.i1];
			IBKMK::Vector3D p1 = planeTri1.m_vertexes[tri.i2];
			IBKMK::Vector3D p2 = planeTri1.m_vertexes[tri.i3];

			IBKMK::Vector3D nTri = (p1-p0).crossProduct(p2-p0);
			nTri.normalize();
			IBKMK::Vector3D nDiff = nTri - s.geometry().normal();
			//double x = nDiff.magnitude();
			if(nDiff.magnitude() > 1.5 ){
				// jetzt stehen die Normalen in unterschiedliche Richtungen
				// erwartet wird, dass die Normalen in die gleiche Richtung zeigen
				// Daher wird die Reihenfolge der Punkte vertauscht
				p2 = planeTri1.m_vertexes[tri.i2];
				p1 = planeTri1.m_vertexes[tri.i3];

			}

			vol +=	p0.m_x * p1.m_y * p2.m_z +
					p2.m_x * p0.m_y * p1.m_z +
					p1.m_x * p2.m_y * p0.m_z

					- p2.m_x * p1.m_y * p0.m_z
					- p0.m_x * p2.m_y * p1.m_z
					- p1.m_x * p0.m_y * p2.m_z;

//			qDebug() << surfaceCounter << "\t" << i << "\t" << volDebug;
			++surfaceCounter;
		}
	}

	vol /= 6;

	if(vol < 0.1) {
		IBK::IBK_Message(IBK::FormatString("Error in volume calculation of room '%1' - Setting Volume to 0.1 m3").arg(m_displayName.toStdString()), IBK::MSG_WARNING, FUNC_ID);
		vol = 0.1;
	}

	// qDebug() << m_displayName << "\t" << vol;

	VICUS::KeywordList::setParameter(m_para,"Room::para_t", VICUS::Room::P_Volume, vol);
}


} // namespace VICUS
