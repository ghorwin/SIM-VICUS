// Test for: https://www.cs.cmu.edu/~quake/triangle.html


#include <iostream>
#include <vector>
#include <string>

extern "C" {
#include "triangle.h"
}

using namespace std;

// Note: triangle uses typedef REAL to be usable for float and double data types

struct Point {
	Point() {}
	Point(REAL x, REAL y) : m_x(x), m_y(y) {}
	double m_x, m_y;
};

struct Triangle {
	Triangle() {}
	unsigned int i1, i2, i3;
};

/*! A class to wrap the natice c-api of triangle.h */
class TriangleWrapper {
public:

	std::vector<Point>		m_vertexes;
	std::vector<Triangle>	m_triangles;

	void operator << (const Point &p) {m_vertexes.push_back(p);}

	void triangulatePolygon();

private:

	// data members needed by triangle
	std::vector<REAL>		m_pointAttributeList;
	std::vector<int>		m_pointMarkerList;
	std::vector<REAL>		m_regionList;

};



int main() {
	cout << "Triangulation test" << endl;

	TriangleWrapper w;
	//w.m_vertexes = { Point(0,0), Point(2,0), Point(2,2),Point(0,2)}; // M shape

	w << Point(0,0);
	w << Point(2,0);
	w << Point(2,2);
	w << Point(2,0);

	w.triangulatePolygon();

	return 0;
}




// here we wrap the triangle call

void TriangleWrapper::triangulatePolygon() {

	// compose input structure
	// Note: storage arrays are plain std::vector<double> of class TriangleWrapper
	//       the struct in only manages a list of pointers for triangle to use
	struct triangulateio in;

	// Note: triangle uses typedef REAL to be usable for float and double data types


	// *** Points ***
	unsigned int n = m_vertexes.size();
	in.numberofpoints = n;
	in.pointlist = (REAL*)m_vertexes.data(); // 0(x)  0(y); 2/0;
	for (unsigned int i=0; i<2*n; i+=2) {
		std::cout << in.pointlist[i] << "\t" << in.pointlist[i+1] << std::endl;
	}

	// *** Point marker list ***
//	m_pointMarkerList.resize(n);

	m_pointMarkerList = std::vector<int>(n,1);
	in.pointmarkerlist = m_pointMarkerList.data();
//	for (unsigned int i=0; i<n; ++i)
//		in.pointmarkerlist[i] = 1; // ??

	// *** Point attributes list ***
	in.numberofpointattributes = 0; // ??
	in.pointattributelist = (REAL*) NULL;
	m_pointAttributeList.resize(n*in.numberofpointattributes);

	//in.pointattributelist = (REAL *) m_pointAttributeList.data();

	//for (unsigned int i=0; i<n; ++i)
	//	in.pointattributelist[i] = m_vertexes[i].m_y; // ??


	// *** Segment attributes list ***
	in.numberofsegments = n;
	std::vector<int> segmentMarkerList{0, 1, 1, 2, 2, 3, 3, 1};
	in.segmentmarkerlist = segmentMarkerList.data();

	in.numberofholes = 0;
	in.numberofregions = 0;

	m_regionList.resize(in.numberofregions*4); // each region has 4 attributes
	in.regionlist = (REAL*) NULL; //m_regionList.data();
	//regionlist "x" "y" "attribute" "max Area" 4 indices
//	in.regionlist[0] = 0.5;
//	in.regionlist[1] = 5.0;
//	in.regionlist[2] = 7.0;            /* Regional attribute (for whole mesh). */
//	in.regionlist[3] = 0.1;            /* Area constraint that will not be used. */

	struct triangulateio out, vorout;

	out.pointlist = (REAL *) NULL;            /* Not needed if -N switch used. */
	/* Not needed if -N switch used or number of point attributes is zero: */
	out.pointattributelist = (REAL *) NULL;
	out.pointmarkerlist = (int *) NULL; /* Not needed if -N or -B switch used. */
	out.trianglelist = (int *) NULL;          /* Not needed if -E switch used. */
	/* Not needed if -E switch used or number of triangle attributes is zero: */
	out.triangleattributelist = (REAL *) NULL;
	out.neighborlist = (int *) NULL;         /* Needed only if -n switch used. */
	/* Needed only if segments are output (-p or -c) and -P not used: */
	out.segmentlist = (int *) NULL;
	/* Needed only if segments are output (-p or -c) and -P and -B not used: */
	out.segmentmarkerlist = (int *) NULL;
	out.edgelist = (int *) NULL;             /* Needed only if -e switch used. */
	out.edgemarkerlist = (int *) NULL;   /* Needed if -e used and -B not used. */

	vorout.pointlist = (REAL *) NULL;        /* Needed only if -v switch used. */
	/* Needed only if -v switch used and number of attributes is not zero: */
	vorout.pointattributelist = (REAL *) NULL;
	vorout.edgelist = (int *) NULL;          /* Needed only if -v switch used. */
	vorout.normlist = (REAL *) NULL;         /* Needed only if -v switch used. */


	triangulate("pzV", &in, &out, NULL);
	for (unsigned int i=0; i<out.numberoftriangles*3; i+=3)
		std::cout << "[ " << out.trianglelist[i] << "," << out.trianglelist[i+1] << "," << out.trianglelist[i+2] << "]\n";


}
