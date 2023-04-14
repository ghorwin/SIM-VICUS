#include "VICUS_Drawing.h"


namespace VICUS {

Drawing::Drawing()
{
	m_layer = std::vector<Layer>();
	m_points = std::vector<Point>();
	m_lines = std::vector<Line>();
	m_polylines = std::vector<PolyLine>();
	m_lwpolylines = std::vector<LWPolyLine>();
}


void Drawing::updatePointer(){
	for(unsigned int i = 0; i < m_points.size(); i++){
		m_points[i].m_parentLayer = findLayerPointer(m_points[i].m_layername);
	}
	for(unsigned int i = 0; i < m_lines.size(); i++){
		m_lines[i].m_parentLayer = findLayerPointer(m_lines[i].m_layername);
	}
	for(unsigned int i = 0; i < m_lwpolylines.size(); i++){
		m_lwpolylines[i].m_parentLayer = findLayerPointer(m_lwpolylines[i].m_layername);
	}
	for(unsigned int i = 0; i < m_polylines.size(); i++){
		m_polylines[i].m_parentLayer = findLayerPointer(m_polylines[i].m_layername);
	}

}


Drawing::Layer* Drawing::findLayerPointer(const QString &layername){
	for(unsigned int i = 0; i < m_layer.size(); i++){
		if(m_layer[i].m_name == layername){
			return &m_layer[i];
		}
	}

	return nullptr;
}


const QColor * Drawing::AbstractDrawingObject::color() const{
	if (m_color.isValid()) {
		return &m_color;
	} else if (m_parentLayer != nullptr && m_parentLayer->m_color.isValid()) {
		return &(m_parentLayer->m_color);
	} else {
		return nullptr;
	}
}




} // namespace VICUS
