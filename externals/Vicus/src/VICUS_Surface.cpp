#include "VICUS_Surface.h"

namespace VICUS {

void Surface::updateColor(const SurfaceType& type){
	switch (type) {
		case SC_Roof :	m_color = QColor(150,50,20,1);		break;
		case SC_Wall :	m_color = QColor(200,200,140,1);	break;
		case SC_Floor :	m_color = QColor(20,50,150,1);		break;
		default :		m_color = QColor(200,200,140,1);	break;
	}
}

}
