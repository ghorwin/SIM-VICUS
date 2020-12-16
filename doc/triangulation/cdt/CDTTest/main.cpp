#include <iostream>

#include <QPolygonF>

#include "CDT.h"
using Triangulation = CDT::Triangulation<double>;

using namespace std;

namespace IBK {
template <typename T>
class point2D{

public:
	/*! Default constructor. */
	point2D() : m_x(T()), m_y(T()) {}
	/*! Constructor (initialises with coordinates 'a' and 'b'). */
	point2D(T a, T b) : m_x(a), m_y(b) {}
	/*! Sets both coordinates. */
	void set(T a, T b) { m_x=a; m_y=b; }

	T m_x;   ///< The X-coordinate.
	T m_y;   ///< The Y-coordinate.
};
}

void setupTriangulation(const QPolygonF &poly, const std::vector<QPolygonF> &holes){

	std::vector<CDT::V2d<double>> vertices;
	std::vector<CDT::Edge> edges;


	//set up points of the outer polyline
	for (auto p : poly)
		vertices.emplace_back(CDT::V2d<double>::make(p.x(),p.y()));

	//set up edges of the outer polyline
	unsigned int vertiSize = vertices.size();
	for(unsigned int i=0; i<vertiSize; ++i)
			edges.push_back(CDT::Edge(i,(i+1)%vertiSize));

	//set up points and edges of the different holes
	for (size_t i=0; i<holes.size(); ++i) {
		for (auto p : holes[i])
			vertices.emplace_back(CDT::V2d<double>::make(p.x(),p.y()));

		for(unsigned int i=vertiSize; i<vertices.size(); ++i){
				if(i==vertices.size()-1)
					edges.push_back(CDT::Edge(i,vertiSize));
				else
					edges.push_back(CDT::Edge(i,i+1));
		}
		vertiSize = vertices.size();
	}

	Triangulation cdt =
		Triangulation(CDT::FindingClosestPoint::ClosestRandom); // Note: we don't want to use boost
	cdt.insertVertices(vertices);
	cdt.insertEdges(edges);
	cdt.eraseOuterTrianglesAndHoles();

	for (unsigned int iTri=0; iTri<cdt.triangles.size(); ++iTri) {

//		for(unsigned int i=0; i<3; ++i)
//			std::cout << cdt.triangles[iTri].vertices[i] << "\t";
		std::cout << std::endl;
		for(unsigned int i=0; i<3; ++i){
			CDT::V2d<double> p = cdt.vertices[cdt.triangles[iTri].vertices[i]].pos;
			std::cout << p.x << "," << p.y << std::endl;
		}
		CDT::V2d<double> p = cdt.vertices[cdt.triangles[iTri].vertices[0]].pos;
		std::cout << p.x << "," << p.y << std::endl;
//		std::cout << std::endl;
	}


}

int main() {

	cout << "TestDirk" << std::endl;


	QPolygonF poly;
	std::vector<QPolygonF>	holes;
	if(false)
	{
		/*
			verwundene Polygone funktionieren nicht.
			Colineare Punkte werden erkannt und mit trianguliert.
		*/

		poly << QPointF(0,0);
		poly << QPointF(1,0);
		poly << QPointF(1,1);
		poly << QPointF(0,1);
		poly << QPointF(0,2);
		poly << QPointF(-1,2);
		poly << QPointF(-1,1);
		poly << QPointF(-1,0);
		poly << QPointF(-2,0);
		poly << QPointF(-2,-1);
		poly << QPointF(2,-1);
	//	poly << QPointF(1,-2);
	}
	else if(false){
		/*
			bei polylines mit gleichen punkten in der polyline
			entstehen degenerierte Dreiecke
			________
			|  /\  |
			| /__\ |
			|______|
		*/
		poly << QPointF(0,0);
		poly << QPointF(4,0);
		poly << QPointF(4,3);
		poly << QPointF(2,3);
		poly << QPointF(3,1);
		poly << QPointF(1,1);
		poly << QPointF(2,3);
		poly << QPointF(0,3);
	}
	else if(true){
		poly << QPointF(0,0);
		poly << QPointF(4,0);
		poly << QPointF(4,3);
		poly << QPointF(0,3);

		QPolygonF hole;
		hole << QPointF(0.5,1);
		hole << QPointF(1,1);
		hole << QPointF(1,2);
		hole << QPointF(0.5,2);

		holes.emplace_back(hole);

		hole.translate(1,0);
		holes.emplace_back(hole);

	}

	//print polygon points
	for (auto p : poly)
		std::cout << p.x() << "," << p.y() << std::endl;


	setupTriangulation(poly, holes);

	return 0;

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
