#include "VICUS_Surface.h"

namespace VICUS {

void Surface::updateColor() {
	// default color for walls
	m_color = QColor(200,200,140,255);
	const double angleForWalls = 0.707;
	// Floor
	if (m_geometry.normal().m_z < -angleForWalls)
		m_color = QColor(20,50,150,255);
	// Roof
	else if (m_geometry.normal().m_z > angleForWalls)
		m_color = QColor(150,50,20,255);
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
