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
    for(size_t i = 0; i < m_points.size(); i++){
        m_points[i].m_parentLayer = Drawing::findLayerPointer(m_points[i].m_layername);
    }
    for(size_t i = 0; i < m_lines.size(); i++){
        m_lines[i].m_parentLayer = Drawing::findLayerPointer(m_lines[i].m_layername);
    }
    for(size_t i = 0; i < m_lwpolylines.size(); i++){
        m_lwpolylines[i].m_parentLayer = Drawing::findLayerPointer(m_lwpolylines[i].m_layername);
    }
    for(size_t i = 0; i < m_polylines.size(); i++){
        m_polylines[i].m_parentLayer = Drawing::findLayerPointer(m_polylines[i].m_layername);
    }
}

Drawing::Layer* Drawing::findLayerPointer(const QString &layername){
    for(size_t i = 0; i < m_layer.size(); i++){
        if(m_layer[i].m_name == layername){
            return &m_layer[i];
        }
    }

    return nullptr;
}



} // namespace VICUS
