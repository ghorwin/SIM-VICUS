
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
#if 0
	// outer
	vertices.push_back(CDT::V2d<double>::make(0,0));
	vertices.push_back(CDT::V2d<double>::make(5,0));
	vertices.push_back(CDT::V2d<double>::make(5,4));
	vertices.push_back(CDT::V2d<double>::make(0,4));
#endif
#if 0
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
#if 0
	//outer bound
	vertices.push_back(CDT::V2d<double>::make(0,0));
	vertices.push_back(CDT::V2d<double>::make(2,0));
	vertices.push_back(CDT::V2d<double>::make(2,2));
	vertices.push_back(CDT::V2d<double>::make(4,2));
	vertices.push_back(CDT::V2d<double>::make(4,0));
	vertices.push_back(CDT::V2d<double>::make(6,0));
	vertices.push_back(CDT::V2d<double>::make(6,6));
	vertices.push_back(CDT::V2d<double>::make(0,6));
#endif
	//outer bound
	vertices.push_back(CDT::V2d<double>::make(0,0));
	vertices.push_back(CDT::V2d<double>::make(5,0));
	vertices.push_back(CDT::V2d<double>::make(5,3));
	vertices.push_back(CDT::V2d<double>::make(0,3));

	std::vector<CDT::Edge> edgeVec;
	for(unsigned int i=0; i<vertices.size(); ++i){

		edgeVec.push_back( CDT::Edge(i, (i+1)%vertices.size()) );
	}

	// hole
	vertices.push_back(CDT::V2d<double>::make(1,1));
	vertices.push_back(CDT::V2d<double>::make(2,1));
	vertices.push_back(CDT::V2d<double>::make(2,2));
	vertices.push_back(CDT::V2d<double>::make(1,2));

	edgeVec.push_back( CDT::Edge(4, 5) );
	edgeVec.push_back( CDT::Edge(5, 6) );
	edgeVec.push_back( CDT::Edge(6, 7) );
	edgeVec.push_back( CDT::Edge(7, 4) );

	// hole
	vertices.push_back(CDT::V2d<double>::make(3,1));
	vertices.push_back(CDT::V2d<double>::make(4,1));
	vertices.push_back(CDT::V2d<double>::make(4,2));
	vertices.push_back(CDT::V2d<double>::make(3,2));

	edgeVec.push_back( CDT::Edge(8, 9) );
	edgeVec.push_back( CDT::Edge(9, 10) );
	edgeVec.push_back( CDT::Edge(10, 11) );
	edgeVec.push_back( CDT::Edge(11, 8) );

#if 0
	edgeVec.push_back( CDT::Edge(4, 5) );
	edgeVec.push_back( CDT::Edge(5, 6) );
	edgeVec.push_back( CDT::Edge(6, 7) );
	edgeVec.push_back( CDT::Edge(7, 4) );
#endif
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
