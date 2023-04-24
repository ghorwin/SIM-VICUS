#include "VICUS_Drawing.h"


namespace VICUS {

Drawing::Drawing()
{
	m_blocks = std::vector<Block>();
	m_layer = std::vector<Layer>();
	m_points = std::vector<Point>();
	m_lines = std::vector<Line>();
	m_polylines = std::vector<PolyLine>();
	m_circles = std::vector<Circle>();
	m_arcs = std::vector<Arc>();
	m_ellipses = std::vector<Ellipse>();
	m_solids = std::vector<Solid>();
}


void Drawing::updatePointer(){
	for(unsigned int i = 0; i < m_points.size(); i++){
		m_points[i].m_parentLayer = findLayerPointer(m_points[i].m_layername);
	}
	for(unsigned int i = 0; i < m_lines.size(); i++){
		m_lines[i].m_parentLayer = findLayerPointer(m_lines[i].m_layername);
	}
	for(unsigned int i = 0; i < m_polylines.size(); i++){
		m_polylines[i].m_parentLayer = findLayerPointer(m_polylines[i].m_layername);
	}
	for(unsigned int i = 0; i < m_circles.size(); i++){
		m_circles[i].m_parentLayer = findLayerPointer(m_circles[i].m_layername);
	}
	for(unsigned int i = 0; i < m_arcs.size(); i++){
		m_arcs[i].m_parentLayer = findLayerPointer(m_arcs[i].m_layername);
	}
	for(unsigned int i = 0; i < m_ellipses.size(); i++){
		m_ellipses[i].m_parentLayer = findLayerPointer(m_ellipses[i].m_layername);
	}
	for(unsigned int i = 0; i < m_solids.size(); i++){
		m_solids[i].m_parentLayer = findLayerPointer(m_solids[i].m_layername);
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
	/* If the object has a color, return it, else use color of parent */
	if (m_color.isValid()) {
		return &m_color;
	} else {
		return &(m_parentLayer->m_color);
	}
}

double Drawing::AbstractDrawingObject::lineWeight() const{
	/* if -1: use weight of layer */
	if (m_lineWeight == -1) {
		if(m_parentLayer->m_lineWeight < 0){
			return 0;
		}
		else{
			return m_parentLayer->m_lineWeight;
		}
	}
	/* if -3: default lineWeight is used
		if -2: lineWeight of block is used. Needs to be modified when blocks
		are implemented */
	else if(m_lineWeight == -3 || m_lineWeight == -2){
		return 0;
	}
	else {
		return m_lineWeight;
	}
}




} // namespace VICUS
