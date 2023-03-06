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

#include <IBKMK_3DCalculations.h>

#include <IBK_messages.h>

#include <tinyxml.h>

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
	for (Surface &childSurf : m_childSurfaces) {
		m_children.push_back(&childSurf);
		childSurf.m_parent = this;
		childSurf.updateParents();
    }
}


void Surface::readXML(const TiXmlElement * element) {
	FUNCID(Surface::readXML);
	// read 3D geometry
	VICUS::Polygon3D poly3D;
	const TiXmlElement * c = element->FirstChildElement();
	while (c) {
		const std::string & cName = c->ValueStr();
		if (cName == "Polygon3D") {
			try {
				poly3D.readXML(c);
			} catch (...) {
				// invalid polygons are just skipped, we don't want to raise an error here!
				IBK::IBK_Message( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
					IBK::FormatString("Error reading Polygon3D tag.") ), IBK::MSG_WARNING, FUNC_ID);
				return;
			}
			// remove Polygon3D element from parent, to avoid getting spammed with "unknown Polygon3D" warning
			const_cast<TiXmlElement *>(element)->RemoveChild(const_cast<TiXmlElement *>(c));
			break;
		}
		c = c->NextSiblingElement();
	}

	readXMLPrivate(element);
	// copy polygon to plane geometry
	std::vector<PlaneGeometry::Hole> holes;
	for(const SubSurface & s : m_subSurfaces)
		holes.push_back(PlaneGeometry::Hole(s.m_id, s.m_polygon2D, false));

	for(const Surface & s : m_childSurfaces) {
		const IBKMK::Vector3D &offset = poly3D.offset();
		const IBKMK::Vector3D &localX = poly3D.localX();
		const IBKMK::Vector3D &localY = poly3D.localY();

		const std::vector<IBKMK::Vector3D> &vertexes = s.polygon3D().vertexes();
		std::vector<IBKMK::Vector2D> holePoints(vertexes.size());

		for(unsigned int j=0; j<vertexes.size(); ++j) {
			IBKMK::planeCoordinates(offset, localX, localY, vertexes[j], holePoints[j].m_x, holePoints[j].m_y);
		}

		holes.push_back(PlaneGeometry::Hole(s.m_id, holePoints, true) );
	}
	// if we didn't get a Polygon3D element, the next call will throw an exception
	m_geometry.setGeometry( poly3D, holes);
}


TiXmlElement * Surface::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = writeXMLPrivate(parent);
	// now add Polygon3D
	m_geometry.polygon3D().writeXML(e);
	return e;
}


void Surface::setPolygon3D(const IBKMK::Polygon3D & polygon) {
	m_geometry.setPolygon((VICUS::Polygon3D)polygon);
}


void Surface::setChildAndSubSurfaces(const std::vector<SubSurface>  & subSurfaces,
									 const std::vector<Surface>     & childSurfaces) {
	m_subSurfaces = subSurfaces;
	m_childSurfaces = childSurfaces;
	std::vector<PlaneGeometry::Hole> holes;
	for (const SubSurface & s : m_subSurfaces)
		holes.push_back(PlaneGeometry::Hole(s.m_id, s.m_polygon2D, false));
	for (const Surface & s : m_childSurfaces) {

		const IBKMK::Vector3D &offset = geometry().offset();
		const IBKMK::Vector3D &localX = geometry().localX();
		const IBKMK::Vector3D &localY = geometry().localY();

		const std::vector<IBKMK::Vector3D> &vertexes = s.polygon3D().vertexes();
		std::vector<IBKMK::Vector2D> holePoints(vertexes.size());

		for(unsigned int j=0; j<vertexes.size(); ++j) {
			IBKMK::planeCoordinates(offset, localX, localY, vertexes[j], holePoints[j].m_x, holePoints[j].m_y);
		}

		holes.push_back(PlaneGeometry::Hole(s.m_id, holePoints, true) );
	}
	m_geometry.setHoles(holes);
}

void Surface::flip() {
	m_geometry.flip(); // the hole polygons have been adjusted here already
	IBK_ASSERT(m_subSurfaces.size() == m_geometry.holes().size());
	for (unsigned int i=0, count=m_subSurfaces.size(); i<count; ++i)
		m_subSurfaces[i].m_polygon2D = m_geometry.holes()[i].m_holeGeometry;
}

void Surface::changeOrigin(unsigned int idx) {
	m_geometry.changeOrigin(idx);
}



} // namespace VICUS
