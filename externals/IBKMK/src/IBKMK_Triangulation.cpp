#include "IBKMK_Triangulation.h"

#include <IBK_messages.h>

#include <CDT.h>

namespace IBKMK {

bool Triangulation::setPoints(const std::vector<IBK::point2D<double> > & points,
							  const std::vector<std::pair<unsigned int, unsigned int> > & edges)
{
//	FUNCID(Triangulation::setPoints);

	CDT::Triangulation<double> cdt(CDT::FindingClosestPoint::ClosestRandom); // Note: we don't want to use boost

	IBK_ASSERT(sizeof(CDT::V2d<double>) == sizeof(IBK::point2D<double>));
	// since IBK::point2D<double> and CDT::V2d<double> are internally the same, we can just re-interpret our original
	// vector as vertex vector
	const std::vector<CDT::V2d<double> > * vertices = reinterpret_cast<	const std::vector<CDT::V2d<double> > * >(&points);

	std::vector<CDT::Edge> edgeVec;
	edgeVec.reserve(edges.size());
	for (const std::pair<unsigned int, unsigned int> & p : edges) {
		edgeVec.push_back( CDT::Edge(p.first, p.second) );
	}

	cdt.insertVertices(*vertices);
	cdt.insertEdges(edgeVec);
	cdt.eraseOuterTrianglesAndHoles();

	// now transfer the triangle

	m_triangles.resize(cdt.triangles.size());
	for (unsigned int i=0; i<cdt.triangles.size(); ++i) {
		const CDT::VerticesArr3 & t = cdt.triangles[i].vertices;
		m_triangles[i] = triangle_t(t[0], t[1], t[2]);
	}

	return true;
}


} // namespace IBKMK

