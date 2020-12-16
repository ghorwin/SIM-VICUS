#include <iostream>

#include "CDT.h"
using Triangulation = CDT::Triangulation<double>;

using namespace std;

int main() {
	cout << "Triangulation convec hull" << endl;

	Triangulation cdt =
		Triangulation(CDT::FindingClosestPoint::ClosestRandom, 10); // Note: we don't want to use boost

	// this is unconstrained
	std::vector<CDT::V2d<double> > vertices;
	vertices.push_back(CDT::V2d<double>::make(0,0));
	vertices.push_back(CDT::V2d<double>::make(2,0));
	vertices.push_back(CDT::V2d<double>::make(2,2));
	vertices.push_back(CDT::V2d<double>::make(1,1));
	vertices.push_back(CDT::V2d<double>::make(0,2));
	cdt.insertVertices(vertices);
	cdt.eraseSuperTriangle();

	for (unsigned int i=0; i<cdt.triangles.size(); ++i) {
		std::cout << "[" << cdt.triangles[i].vertices[0] << "," << cdt.triangles[i].vertices[1] << "," << cdt.triangles[i].vertices[2] << "]" << std::endl;
	}


	cout << "Triangulation concave hull" << endl;
	// now with edge constraints
	cdt =
		Triangulation(CDT::FindingClosestPoint::ClosestRandom, 10); // Note: we don't want to use boost

	std::vector<CDT::Edge> edges;
	edges.push_back(CDT::Edge(0,1));
	edges.push_back(CDT::Edge(1,2));
	edges.push_back(CDT::Edge(2,3));
	edges.push_back(CDT::Edge(3,4));
	edges.push_back(CDT::Edge(4,0));
	cdt.insertVertices(vertices);
	cdt.insertEdges(edges);
	cdt.eraseOuterTriangles();
	for (unsigned int i=0; i<cdt.triangles.size(); ++i) {
		std::cout << "[" << cdt.triangles[i].vertices[0] << "," << cdt.triangles[i].vertices[1] << "," << cdt.triangles[i].vertices[2] << "]" << std::endl;
	}

	cout << "Triangulation with holes" << endl;
	// now with edge constraints
	cdt =
		Triangulation(CDT::FindingClosestPoint::ClosestRandom, 10); // Note: we don't want to use boost

	vertices.clear();
	edges.clear();

	// construct a box with inner box

	// outer box vertexes
	vertices.push_back(CDT::V2d<double>::make(0,0));
	vertices.push_back(CDT::V2d<double>::make(3,0));
	vertices.push_back(CDT::V2d<double>::make(3,3));
	vertices.push_back(CDT::V2d<double>::make(0,3));

	// inner box vertexes
	vertices.push_back(CDT::V2d<double>::make(1,1));
	vertices.push_back(CDT::V2d<double>::make(1,2));
	vertices.push_back(CDT::V2d<double>::make(2,2));
	vertices.push_back(CDT::V2d<double>::make(2,1));

	edges.clear();
	// edges of the outer box
	edges.push_back(CDT::Edge(0,1));
	edges.push_back(CDT::Edge(1,2));
	edges.push_back(CDT::Edge(2,3));
	edges.push_back(CDT::Edge(3,0));
	// edges of the inner box
	edges.push_back(CDT::Edge(4,5));
	edges.push_back(CDT::Edge(5,6));
	edges.push_back(CDT::Edge(6,7));
	edges.push_back(CDT::Edge(7,4));

	cdt.insertVertices(vertices);
	cdt.insertEdges(edges);
	cdt.eraseOuterTrianglesAndHoles();
	std::cout << cdt.triangles.size();
	for (unsigned int i=0; i<cdt.triangles.size(); ++i) {
		std::cout << "[" << cdt.triangles[i].vertices[0] << "," << cdt.triangles[i].vertices[1] << "," << cdt.triangles[i].vertices[2] << "]" << std::endl;
	}


	return 0;
}
