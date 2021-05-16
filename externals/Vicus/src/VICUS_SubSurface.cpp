#include "VICUS_SubSurface.h"

namespace VICUS {

SubSurface SubSurface::clone() const {
	SubSurface r(*this); // create new SubSurface with same unique ID
	Object & o = r;
	(Object&)r = o.clone(); // assign new ID only
	return r;
}


void SubSurface::updateColor() {
	if (m_subSurfaceComponentInstance == nullptr) {
		// no subsurface assigned -> dark gray
		m_color = QColor(64,64,64,255);
	}
	else {
		// TODO : depending on assigned subsurface type, select suitable color
		// for now always transparent blue
		m_color = QColor(96,96,255,128);
	}
}

} // namespace VICUS
