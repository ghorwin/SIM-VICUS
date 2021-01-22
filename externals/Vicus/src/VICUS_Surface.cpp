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

} // namespace VICUS
