#include "VICUS_SubSurface.h"

namespace VICUS {

void SubSurface::updateColor() {
	// default color for walls
	m_color = QColor(200,200,140,1);
	const double angleForWalls = 0.707;
}

} // namespace VICUS
