
#include <CDT/CDT.h>
#include <IBK_assert.h>

#include <vector>
#include <IBK_point.h>

int main() {

	CDT::Triangulation<double> cdt(CDT::FindingClosestPoint::ClosestRandom); // Note: we don't want to use boost

	IBK_ASSERT(sizeof(CDT::V2d<double>) == sizeof(IBK::point2D<double>));

	// since IBK::point2D<double> and CDT::V2d<double> are internally the same, we can just re-interpret our original
	// vector as vertex vector
	std::vector<CDT::V2d<double> > vertices;

	// outer
	vertices.push_back(CDT::V2d<double>::make(0,0));
	vertices.push_back(CDT::V2d<double>::make(5,0));
	vertices.push_back(CDT::V2d<double>::make(5,4));
	vertices.push_back(CDT::V2d<double>::make(0,4));

#if 1
	// hole
	vertices.push_back(CDT::V2d<double>::make(1,1));
	vertices.push_back(CDT::V2d<double>::make(4,1));
	vertices.push_back(CDT::V2d<double>::make(4,2));
	vertices.push_back(CDT::V2d<double>::make(1,2));
#endif

#if 0
	// loch am rand geht
	vertices.push_back(CDT::V2d<double>::make(1,1));
	vertices.push_back(CDT::V2d<double>::make(5,0));
	vertices.push_back(CDT::V2d<double>::make(5,4));
	vertices.push_back(CDT::V2d<double>::make(1,2));
#endif

#if 0
	// vollflächig geht nicht!
	vertices.push_back(CDT::V2d<double>::make(0,0));
	vertices.push_back(CDT::V2d<double>::make(5,0));
	vertices.push_back(CDT::V2d<double>::make(5,4));
	vertices.push_back(CDT::V2d<double>::make(0,4));
#endif

	std::vector<CDT::Edge> edgeVec;
	edgeVec.push_back( CDT::Edge(0, 1) );
	edgeVec.push_back( CDT::Edge(1, 2) );
	edgeVec.push_back( CDT::Edge(2, 3) );
	edgeVec.push_back( CDT::Edge(3, 0) );

	edgeVec.push_back( CDT::Edge(4, 5) );
	edgeVec.push_back( CDT::Edge(5, 6) );
	edgeVec.push_back( CDT::Edge(6, 7) );
	edgeVec.push_back( CDT::Edge(7, 4) );

	cdt.insertVertices(vertices);
	cdt.insertEdges(edgeVec);
	cdt.eraseOuterTrianglesAndHoles();

	// now transfer the triangle
	for (unsigned int i=0; i<cdt.triangles.size(); ++i) {
		const CDT::VerticesArr3 & t = cdt.triangles[i].vertices;
		std::cout << t[0] << "," << t[1] << "," << t[2] << std::endl;
	}

	return 0;
}
