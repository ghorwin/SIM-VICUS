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

#include "VICUS_Surface.h"

#include "IBKMK_3DCalculations.h"

namespace VICUS {

void Surface::initializeColorBasedOnInclination() {
	// default color for walls
	m_displayColor = QColor(200,200,140,255);
	const double angleForWalls = 0.35; // only very vertical planes are treated as walls
	// Floor
	if (m_geometry.normal().m_z < -angleForWalls)
		m_displayColor = QColor("#566094");
	// Roof
	else if (m_geometry.normal().m_z > angleForWalls)
		m_displayColor = QColor(150,50,20,255);
}


Surface Surface::clone() const{
	Surface r(*this); // create new surface with same unique ID
	Object & o = r;
	(Object&)r = o.clone(); // assign new ID only
	return r;
}


void Surface::updateParents() {
	m_children.clear();
	for (SubSurface & sub : m_subSurfaces) {
		m_children.push_back(&sub);
		sub.m_parent = this;
	}
}


void Surface::readXML(const TiXmlElement * element) {
	readXMLPrivate(element);
	// copy polygon to plane geometry
	std::vector<Polygon2D> holes;
	for (const SubSurface & s : m_subSurfaces)
		holes.push_back(s.m_polygon2D);
	m_geometry.setGeometry( m_polygon3D, holes);
}


TiXmlElement * Surface::writeXML(TiXmlElement * parent) const {
	return writeXMLPrivate(parent);
}


void Surface::setPolygon3D(const Polygon3D & polygon) {
	m_polygon3D = polygon;
	m_geometry.setPolygon(polygon);
}


void Surface::setSubSurfaces(const std::vector<SubSurface> & subSurfaces) {
	m_subSurfaces = subSurfaces;
	std::vector<Polygon2D> holes;
	for (const SubSurface & s : subSurfaces)
		holes.push_back(s.m_polygon2D);
	m_geometry.setHoles(holes);
}


void Surface::flip() {

	// TODO  : Dirk

	// compute 3D vertex coordinates of all subsurfaces
	// flip polygon
	// flip subsurface polygons (3D vertex variants)
	// recompute 2D vertex coordinates for all subsurfaces

	// we cache the sub surfaces
	std::vector<std::vector<IBKMK::Vector3D>> copiedSubSurf3D;

	const std::vector<IBKMK::Vector3D> &vertexes = m_geometry.polygon().vertexes();
	std::vector<IBKMK::Vector3D> newVertexes;

	// updated subsurfaces
	std::vector<SubSurface> newSubSurfaces (m_subSurfaces.size() );

	// cache 3D Points of SubSurfaces
	for ( unsigned int i = 0; i<m_subSurfaces.size(); ++i) {
		copiedSubSurf3D.push_back(std::vector<IBKMK::Vector3D>() );
		const SubSurface &sub = m_subSurfaces[i];

		for ( unsigned	int j = 0; j<sub.m_polygon2D.vertexes().size(); ++j ) {
			const IBKMK::Vector2D &poly2D = sub.m_polygon2D.vertexes()[j];
			IBKMK::Vector3D v = vertexes[0] + poly2D.m_x * geometry().localX() + poly2D.m_y * geometry().localY();
			copiedSubSurf3D[i].push_back(v);
		}
	}

	// we generate the new Polygon3D
	for (unsigned int i=(unsigned int)vertexes.size(); i>0; --i)
		newVertexes.push_back(vertexes[i-1]);

	// construct the new Polygon3D
	setPolygon3D(newVertexes);

	std::vector<SubSurface> newSubSurf(m_subSurfaces.size());

	// we update the subsurfaces
	for ( unsigned int i=0; i<copiedSubSurf3D.size(); ++i ) {

		Q_ASSERT(m_subSurfaces.size() == copiedSubSurf3D.size() );

		// we construct a new Polygon2D to save calculated points
		Polygon2D newPoly2D;

		for ( unsigned int j=0; j<copiedSubSurf3D[i].size(); ++j ) {

			Q_ASSERT(m_subSurfaces[i].m_polygon2D.vertexes().size() == copiedSubSurf3D[i].size() );

			// we calculate our new points
			IBKMK::Vector2D v;

			if (!IBKMK::planeCoordinates(newVertexes[0], geometry().localX(), geometry().localY(), copiedSubSurf3D[i][j], v.m_x, v.m_y, 1e-4))
				return;

			newPoly2D.addVertex(v);

		}

		// new we finally updated the polygon2D in the subsurface
		m_subSurfaces[i].m_polygon2D = newPoly2D;
	}
}


} // namespace VICUS
