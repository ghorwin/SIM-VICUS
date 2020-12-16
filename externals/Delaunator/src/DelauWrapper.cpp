#include "DelauWrapper.h"

#include "delaunator.hpp"

void DelauWrapper::setPoints(const QPolygonF & poly)
{
	for (auto p : poly) {
		m_coords.push_back(p.x());
		m_coords.push_back(p.y());
	}
}

void DelauWrapper::getTriangles(std::vector<QPolygonF> &tris)
{
	if(m_coords.empty())
		return;

	delaunator::Delaunator delau(m_coords);

	for (size_t i=0; i<delau.triangles.size(); i+=3) {
		QPolygonF tri;
		for (size_t j=0; j<3; ++j)
			tri << QPointF(m_coords[2*i], m_coords[2+i+1]);
		tris.push_back(tri);
	}

}

void DelauWrapper::getTriangleIdx(std::vector<size_t> &triIdx)
{
	if(m_coords.empty())
		return;

	delaunator::Delaunator delau(m_coords);

	triIdx = delau.triangles;

}
