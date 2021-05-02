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


void Surface::computeGeometry() {
	// copy polygon to plane geometry
	m_geometry.setPolygon( m_polygon3D );
	std::vector<Polygon2D> holes;
	for (const SubSurface & ssurf : m_subSurfaces)
		holes.push_back(ssurf.m_geometry);
	m_geometry.setHoles( holes );
}


} // namespace VICUS
