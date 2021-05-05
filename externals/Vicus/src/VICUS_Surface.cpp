#include "VICUS_Surface.h"

namespace VICUS {

void Surface::updateColor() {
	// default color for walls
	m_color = QColor(200,200,140,1);
	const double angleForWalls = 0.707;
	// Floor
	if (m_geometry.normal().m_z < -angleForWalls)
		m_color = QColor(20,50,150,1);
	// Roof
	else if (m_geometry.normal().m_z > angleForWalls)
		m_color = QColor(150,50,20,1);
}


void Surface::readXML(const TiXmlElement * element) {
	readXMLPrivate(element);
	computeGeometry();
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


void Surface::computeGeometry() {
	// copy polygon to plane geometry
	m_geometry.setPolygon( m_polygon3D );
	setSubSurfaces(m_subSurfaces);
}


void Surface::flip() {
	// TODO  : Dirk

	// compute 3D vertex coordinates of all subsurfaces
	// flip polygon
	// flip subsurface polygons (3D vertex variants)
	// recompute 2D vertex coordinates for all subsurfaces

}


} // namespace VICUS
