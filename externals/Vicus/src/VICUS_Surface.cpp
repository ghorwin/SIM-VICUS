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

	if ( !geometry().isValid() && polygon3D().vertexes().size() > 2 )
		healGeometry(m_polygon3D.vertexes());
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
	m_polygon3D.setRotation(-1.0*m_polygon3D.normal(), m_polygon3D.localX());
}


} // namespace VICUS
