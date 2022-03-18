/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "Vic3DGeometryHelpers.h"

#include <IBKMK_Triangulation.h>
#include <SV_Conversions.h>
#include <VICUS_NetworkLine.h>

#include <QQuaternion>

#include "SVSettings.h"
#include "Vic3DConstants.h"

namespace Vic3D {


#define CYLINDER_SEGMENTS 16
#define SPHERE_SEGMENTS 8

// *** Primitives ***

void addPlane(const VICUS::PlaneTriangulationData & g, const QColor & col,
			  unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
			  std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData, std::vector<GLuint> & indexBufferData,
			  bool inverted)
{
	// add vertex data to buffers
	unsigned int nVertexes = g.m_vertexes.size();
	// insert count vertexes
	vertexBufferData.resize(vertexBufferData.size()+nVertexes);
	colorBufferData.resize(colorBufferData.size()+nVertexes);
	// set data
	QVector3D n = IBKVector2QVector(g.m_normal);
	if (inverted)
		n *= -1;
	for (unsigned int i=0; i<nVertexes; ++i) {
		vertexBufferData[currentVertexIndex + i].m_coords = IBKVector2QVector(g.m_vertexes[i]);
		vertexBufferData[currentVertexIndex + i].m_normal = n;
		colorBufferData[currentVertexIndex  + i] = col;
	}

	unsigned int triangleIndexCount = g.m_triangles.size()*3;
	indexBufferData.resize(indexBufferData.size()+triangleIndexCount);
	// add all triangles

	for (const IBKMK::Triangulation::triangle_t & t : g.m_triangles) {
		if (inverted) {
			indexBufferData[currentElementIndex    ] = currentVertexIndex + t.i1;
			indexBufferData[currentElementIndex + 1] = currentVertexIndex + t.i3;
			indexBufferData[currentElementIndex + 2] = currentVertexIndex + t.i2;
		}
		else {
			indexBufferData[currentElementIndex    ] = currentVertexIndex + t.i1;
			indexBufferData[currentElementIndex + 1] = currentVertexIndex + t.i2;
			indexBufferData[currentElementIndex + 2] = currentVertexIndex + t.i3;
		}
		// advance index in element/index buffer
		currentElementIndex += 3;
	}

	// index in element/index buffer has already been advanced

	// finally advance vertex index
	currentVertexIndex += nVertexes;
}


void addPlaneAsStrip(const IBKMK::Vector3D & a, const IBKMK::Vector3D & b, const IBKMK::Vector3D & d, const QColor & col,
					 unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
					 std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData, std::vector<GLuint> & indexBufferData)
{
	FUNCID(addPlaneAsStrip);
	IBKMK::Polygon3D p(VICUS::Polygon2D::T_Rectangle, a, b, d);
	if (!p.isValid())
		throw IBK::Exception("Invalid polygon.", FUNC_ID);
	VICUS::PlaneGeometry g(p);

	// add vertex data to buffers
	unsigned int nVertexes = g.triangulationData().m_vertexes.size();
	// insert count vertexes
	vertexBufferData.resize(vertexBufferData.size()+nVertexes);
	colorBufferData.resize(colorBufferData.size()+nVertexes);
	// set data
	QVector3D n = IBKVector2QVector(g.normal());
	for (unsigned int i=0; i<nVertexes; ++i) {
		vertexBufferData[currentVertexIndex + i].m_coords = IBKVector2QVector(g.triangulationData().m_vertexes[i]);
		vertexBufferData[currentVertexIndex + i].m_normal = n;
		colorBufferData[currentVertexIndex  + i] = col;
	}


	// 5 elements (4 triangle strip indexs + 1 stop index)
	indexBufferData.resize(indexBufferData.size()+5);

	// anti-clock-wise winding order for all triangles in strip
	// 0, 1, 2
	// 2, 3, 0
	indexBufferData[currentElementIndex    ] = currentVertexIndex + 0;
	indexBufferData[currentElementIndex + 1] = currentVertexIndex + 1;
	indexBufferData[currentElementIndex + 2] = currentVertexIndex + 3;
	indexBufferData[currentElementIndex + 3] = currentVertexIndex + 2;
	indexBufferData[currentElementIndex + 4] = VIC3D_STRIP_STOP_INDEX;

	// advance index in element/index buffer
	currentElementIndex += 5;
}


void addPlane(const VICUS::PlaneTriangulationData & g, unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
			  std::vector<VertexC> & vertexBufferData, std::vector<GLuint> & indexBufferData)
{
	// add vertex data to buffers
	unsigned int nVertexes = g.m_vertexes.size();
	// insert count vertexes
	vertexBufferData.resize(vertexBufferData.size()+nVertexes);
	// set data
	for (unsigned int i=0; i<nVertexes; ++i)
		vertexBufferData[currentVertexIndex + i].m_coords = IBKVector2QVector(g.m_vertexes[i]);

	unsigned int triangleIndexCount = g.m_triangles.size()*3;
	indexBufferData.resize(indexBufferData.size()+triangleIndexCount);
	// add all triangles

	for (const IBKMK::Triangulation::triangle_t & t : g.m_triangles) {
		indexBufferData[currentElementIndex    ] = currentVertexIndex + t.i1;
		indexBufferData[currentElementIndex + 1] = currentVertexIndex + t.i2;
		indexBufferData[currentElementIndex + 2] = currentVertexIndex + t.i3;
		// advance index in element/index buffer
		currentElementIndex += 3;
	}

	// finally advance vertex index
	currentVertexIndex += nVertexes;
}


void updateColors(const VICUS::PlaneTriangulationData & g, const QColor & col, unsigned int & currentVertexIndex, std::vector<ColorRGBA> & colorBufferData) {
	unsigned int nvert = g.m_vertexes.size();
	// add all vertices to buffer
	for (unsigned int i=0; i<nvert; ++i)
		colorBufferData[currentVertexIndex + i] = col;
	// finally advance buffer indexes
	currentVertexIndex += nvert;
}


// ** Cylinders **

void addCylinder(const IBKMK::Vector3D & p1, const IBKMK::Vector3D & p2, const QColor & c, double radius,
				 unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
				 std::vector<Vertex> & vertexBufferData,
				 std::vector<ColorRGBA> & colorBufferData,
				 std::vector<GLuint> & indexBufferData, bool closed)
{
	// THIS IS FOR DRAWING TRIANGLE STRIPS

	// we generate vertices for a cylinder starting at 0,0,0 and extending to 1,0,0 (x-axis is the rotation axis)

	// after each generated vertex, it is scaled, rotated into position, and translated to p1

#define PI_CONST 3.14159265

	// our vertices are numbered 0, 1, 2 with odd vertices at x=0, and even vertices at x=1

	IBKMK::Vector3D cylinderAxis = p2-p1;
	double L = cylinderAxis.magnitude();

	// this is the rotation matrix to be applied to each generated vertex and normal vector
	QQuaternion rot = QQuaternion::rotationTo(QVector3D(1,0,0), IBKVector2QVector(cylinderAxis));
	QVector3D trans = IBKVector2QVector(p1);

	// add the first two vertices (which are also the last)

	unsigned int nSeg = CYLINDER_SEGMENTS; // number of segments to generate
	// insert nSeg*2 vertexes
	vertexBufferData.resize(vertexBufferData.size() + nSeg*2);
	colorBufferData.resize(colorBufferData.size() + nSeg*2);
	// (nSeg+1)*2 + 1 element indexes ((nSeg+1)*2 for the triangle strip, 1 primitive restart index)
	indexBufferData.resize(indexBufferData.size() + (nSeg+1)*2 + 1);

	unsigned int vertexIndexStart = currentVertexIndex;

	// insert vertexes, 2 per segment
	for (unsigned int i=0; i<nSeg; ++i, currentVertexIndex += 2) {
		double angle = -2*PI_CONST*i/nSeg;
		double ny = std::cos(angle);
		double y = ny*radius;
		double nz = std::sin(angle);
		double z = nz*radius;

		// coordinates are rotated and translated
		vertexBufferData[currentVertexIndex    ].m_coords = rot.rotatedVector(QVector3D(0, y, z)) + trans;
		vertexBufferData[currentVertexIndex + 1].m_coords = rot.rotatedVector(QVector3D(L, y, z)) + trans;
		// normals are just rotated
		vertexBufferData[currentVertexIndex    ].m_normal = rot.rotatedVector(QVector3D(0, ny, nz));
		vertexBufferData[currentVertexIndex + 1].m_normal = rot.rotatedVector(QVector3D(0, ny, nz));

		// add colors
		colorBufferData[currentVertexIndex     ] = c;
		colorBufferData[currentVertexIndex  + 1] = c;
	}

	// now add elements
	// the cylinder mesh has triangles (0 1 2) (1 2 3) (2 3 4) ...
	// so we just add all vertex indexes one-by-one
	for (unsigned int i=0; i<nSeg*2; ++i, ++currentElementIndex)
		indexBufferData[currentElementIndex] = i + vertexIndexStart;

	// finally add first two vertices again
	indexBufferData[currentElementIndex++] = vertexIndexStart;
	indexBufferData[currentElementIndex++] = vertexIndexStart+1;
	indexBufferData[currentElementIndex++] = VIC3D_STRIP_STOP_INDEX; // set stop index

	// if a closed cylinder is expected, add the front and back facing plates
	if (closed) {
		// we need 2*nSeg more vertexes + 2 for the centers, because the normal vectors point in different direction
		vertexBufferData.resize(vertexBufferData.size() + 2*nSeg + 2);
		colorBufferData.resize(colorBufferData.size() + 2*nSeg + 2);
		vertexIndexStart = currentVertexIndex;

		// front facing plate
		vertexBufferData[currentVertexIndex].m_coords = rot.rotatedVector(QVector3D(0, 0, 0)) + trans;
		vertexBufferData[currentVertexIndex].m_normal = rot.rotatedVector(QVector3D(-1, 0, 0)) + trans;
		colorBufferData[currentVertexIndex] = c;

		++currentVertexIndex;
		for (unsigned int i=0; i<nSeg; ++i, ++currentVertexIndex) {
			double angle = -2*PI_CONST*i/nSeg;
			double ny = std::cos(angle);
			double y = ny*radius;
			double nz = std::sin(angle);
			double z = nz*radius;
			vertexBufferData[currentVertexIndex].m_coords = rot.rotatedVector(QVector3D(0, y, z)) + trans;
			vertexBufferData[currentVertexIndex].m_normal = rot.rotatedVector(QVector3D(-1, 0, 0)) + trans;
			colorBufferData[currentVertexIndex] = c;
		}

		// rear facing plate
		vertexBufferData[currentVertexIndex].m_coords = rot.rotatedVector(QVector3D(L, 0, 0)) + trans;
		vertexBufferData[currentVertexIndex].m_normal = rot.rotatedVector(QVector3D(1, 0, 0)) + trans;
		colorBufferData[currentVertexIndex] = c;

		++currentVertexIndex;
		for (unsigned int i=0; i<nSeg; ++i, ++currentVertexIndex) {
			double angle = 2*PI_CONST*i/nSeg; // mind different rotation direction than for front-facing plate
			double ny = std::cos(angle);
			double y = ny*radius;
			double nz = std::sin(angle);
			double z = nz*radius;
			vertexBufferData[currentVertexIndex].m_coords = rot.rotatedVector(QVector3D(L, y, z)) + trans;
			vertexBufferData[currentVertexIndex].m_normal = rot.rotatedVector(QVector3D(1, 0, 0)) + trans;
			colorBufferData[currentVertexIndex] = c;
		}

		// also more indexes, since we render with triangle strips, and we have nSeg Vertexes, that makes
		// 2*(nSeg+1) indexes plus one stop index and that 2 times for either side of the cylinder
		indexBufferData.resize(indexBufferData.size() + 2*(3*nSeg + 2));

		// generate the sequence 0 1 2   0 2 3   0 3 4   0 4 1    0 stop
		for (unsigned int i=0; i<nSeg; ++i, currentElementIndex +=3) {

			indexBufferData[currentElementIndex  ] = vertexIndexStart; // 0
			indexBufferData[currentElementIndex+1] = vertexIndexStart + 1 + i; // 1
			indexBufferData[currentElementIndex+2] = vertexIndexStart + 1 + (1 + i) % nSeg; // 2
		}

		indexBufferData[currentElementIndex++] = vertexIndexStart; // 0
		indexBufferData[currentElementIndex++] = VIC3D_STRIP_STOP_INDEX; // stop index

		vertexIndexStart += nSeg + 1;
		// generate the sequence 0 1 2   0 2 3   0 3 4   0 4 1    0 stop
		for (unsigned int i=0; i<nSeg; ++i, currentElementIndex +=3) {

			indexBufferData[currentElementIndex  ] = vertexIndexStart; // 0
			indexBufferData[currentElementIndex+1] = vertexIndexStart + 1 + i; // 1
			indexBufferData[currentElementIndex+2] = vertexIndexStart + 1 + (1 + i) % nSeg; // 2
		}

		indexBufferData[currentElementIndex++] = vertexIndexStart; // 0
		indexBufferData[currentElementIndex++] = VIC3D_STRIP_STOP_INDEX; // stop index


	} // if closed
}


void addCylinder(const IBKMK::Vector3D & p1, const IBKMK::Vector3D & p2, double radius,
				 unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
				 std::vector<VertexC> & vertexBufferData, std::vector<GLuint> & indexBufferData)
{
	// we generate vertices for a cylinder starting at 0,0,0 and extending to 1,0,0 (x-axis is the rotation axis)

	// after each generated vertex, it is scaled, rotated into position, and translated to p1

#define PI_CONST 3.14159265

	// our vertices are numbered 0, 1, 2 with odd vertices at x=0, and even vertices at x=1

	IBKMK::Vector3D cylinderAxis = p2-p1;
	double L = cylinderAxis.magnitude();

	// this is the rotation matrix to be applied to each generated vertex and normal vector
	QQuaternion rot = QQuaternion::rotationTo(QVector3D(1,0,0), IBKVector2QVector(cylinderAxis));
	QVector3D trans = IBKVector2QVector(p1);

	// add the first two vertices (which are also the last)

	unsigned int nSeg = CYLINDER_SEGMENTS; // number of segments to generate
	// insert nSeg*2 vertexes
	vertexBufferData.resize(vertexBufferData.size() + nSeg*2);
	// (nSeg*3*2 element indexes (2 triangls per segment)
	indexBufferData.resize(indexBufferData.size() + nSeg*6);

	unsigned int vertexIndexStart = currentVertexIndex;

	// insert vertexes, 2 per segment
	for (unsigned int i=0; i<nSeg; ++i, currentVertexIndex += 2) {
		double angle = 2*PI_CONST*i/nSeg;
		double ny = std::cos(angle);
		double y = ny*radius;
		double nz = std::sin(angle);
		double z = nz*radius;

		// coordinates are rotated and translated
		vertexBufferData[currentVertexIndex    ].m_coords = rot.rotatedVector(QVector3D(0, y, z)) + trans;
		vertexBufferData[currentVertexIndex + 1].m_coords = rot.rotatedVector(QVector3D(L, y, z)) + trans;
	}

	// now add elements
	for (unsigned int i=0; i<nSeg; ++i, currentElementIndex+=6) {
		indexBufferData[currentElementIndex  ] = 2*i           + vertexIndexStart;
		indexBufferData[currentElementIndex+1] = 2*i + 1       + vertexIndexStart;
		indexBufferData[currentElementIndex+2] = (2*i + 2) % (nSeg*2) + vertexIndexStart;
		indexBufferData[currentElementIndex+3] = 2*i + 1       + vertexIndexStart;
		indexBufferData[currentElementIndex+4] = (2*i + 3) % (nSeg*2) + vertexIndexStart;
		indexBufferData[currentElementIndex+5] = (2*i + 2) % (nSeg*2) + vertexIndexStart;
	}
}


void updateCylinderColors(const QColor & c, unsigned int & currentVertexIndex, std::vector<ColorRGBA> & colorBufferData) {
	unsigned int nSeg = CYLINDER_SEGMENTS; // number of segments to generate

	// insert vertexes, 2 per segment
	for (unsigned int i=0; i<nSeg; ++i, currentVertexIndex += 2) {
		// set colors
		colorBufferData[currentVertexIndex     ] = c;
		colorBufferData[currentVertexIndex  + 1] = c;
	}
}


// ** Spheres **

void addSphere(const IBKMK::Vector3D & p, const QColor & c, double radius,
			   unsigned int & currentVertexIndex,
			   unsigned int & currentElementIndex,
			   std::vector<Vertex> & vertexBufferData,
			   std::vector<ColorRGBA> & colorBufferData,
			   std::vector<GLuint> & indexBufferData)
{
	// THIS IS FOR DRAWING TRIANGLE STRIPS

	QVector3D trans = IBKVector2QVector(p);

	unsigned int nSeg = SPHERE_SEGMENTS; // number of segments to split 180° into
	unsigned int nSeg2 = SPHERE_SEGMENTS*2; // number of segments to split 360° into

	// unfolded sphere mesh has nSeg*nSeg2 squares, and nSeg+1 rings
	vertexBufferData.resize(vertexBufferData.size() + (nSeg+1)*nSeg2);
	colorBufferData.resize(colorBufferData.size() + (nSeg+1)*nSeg2);

	unsigned int vertexStart = currentVertexIndex;
	// nSeg triangle strips
	indexBufferData.resize(indexBufferData.size() + nSeg*(2*(nSeg2+1) + 1)); // Mind: add 2 indexes for each degenerated triangle per ring

	// now generate the vertexes (nSeg vertexes per circle)
	for (unsigned int i=0; i<=nSeg; ++i) {
		double beta = PI_CONST*i/nSeg;
		double flat_radius = std::sin(beta)*radius;
		double nx = std::cos(beta);
		double x = nx*radius;

		for (unsigned int j=0; j<nSeg2; ++j, ++currentVertexIndex) {
			double angle = (double)(j + 0.5*i)/nSeg2;
			angle *= 2*PI_CONST;
			double ny = std::cos(angle);
			double y = ny*flat_radius;
			double nz = std::sin(angle);
			double z = nz*flat_radius;

			vertexBufferData[currentVertexIndex].m_coords = QVector3D(x, y, z) + trans;
			vertexBufferData[currentVertexIndex].m_normal = QVector3D(x, y, z).normalized();
			colorBufferData[currentVertexIndex] = c;
		}
	}

	// indexes are genererated row-by-row
	// The unrolled grid looks like this, with the first vertex colum repeated to show the wrap-around
	// 8   9  10  11   8
	// 4   5   6   7   4
	// 0   1   2   3   0
	//
	// The triangles in the bottom-most row are then: (0 4 1) (4 1 5) (1 5 2) ... (3 7 0) (7 0 4)   ... (4 4 8 degenerated)
	// and in the next row                            (4 8 5) (8 5 9)
	//
	// The strip in the first row is 0 4 1 5 2 6 3 7 0 4  followed by 4 8   = 2*nSeg2 + 2


	for (unsigned int i=0; i<nSeg; ++i) {
		unsigned int topCircleVertexStart = (i+1)*nSeg2;  // i = 0 -> 4
		unsigned int bottomCircleVertexStart = i*nSeg2;   // i = 0 -> 0

		// we add always 2 triangles
		for (unsigned int j=0; j<=nSeg2; ++j, currentElementIndex += 2) {
			// add 0 4 ... 0 4
			indexBufferData[currentElementIndex  ] = vertexStart + bottomCircleVertexStart + (j % nSeg2);
			indexBufferData[currentElementIndex+1] = vertexStart + topCircleVertexStart + (j % nSeg2);
		}

		// add stop index
		indexBufferData[currentElementIndex++] = VIC3D_STRIP_STOP_INDEX; // repeat last vertex once to get two degenerate triangles
	}
}


void addSphere(const IBKMK::Vector3D & p, double radius,
			   unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
			   std::vector<VertexC> & vertexBufferData, std::vector<GLuint> & indexBufferData)
{

	// THIS IS FOR THE WIREFRAME OBJECT - Draw Triangles mode!

	QVector3D trans = IBKVector2QVector(p);

	unsigned int nSeg = SPHERE_SEGMENTS; // number of segments to split 180° into
	unsigned int nSeg2 = nSeg*2; // number of segments to split 360° into

	// unfolded sphere mesh has nSeg*nSeg2 squares, and nSeg+1 rings
	vertexBufferData.resize(vertexBufferData.size() + (nSeg+1)*nSeg2);
	// two triangles per square, nSeg2 squares per row, nSeg rows
	indexBufferData.resize(indexBufferData.size() + nSeg*nSeg2*2*3); // Mind: we draw in "draw triangles" mode in wirefram

	unsigned int vertexStart = currentVertexIndex;

	// now generate the vertexes (nSeg vertexes per circle)
	for (unsigned int i=0; i<=nSeg; ++i) {
		double beta = PI_CONST*i/nSeg;
		double flat_radius = std::sin(beta)*radius;
		double nx = std::cos(beta);
		double x = nx*radius;

		for (unsigned int j=0; j<nSeg2; ++j, ++currentVertexIndex) {
			// Mind the negative sign, since we look at the mesh from the positve x-axis towards the negative axis
			// and want the mesh to loop clock-wise around x-axis from this view point
			double angle = -(double)(j + 0.5*i)/nSeg2;
			angle *= 2*PI_CONST;
			double ny = std::cos(angle);
			double y = ny*flat_radius;
			double nz = std::sin(angle);
			double z = nz*flat_radius;

			vertexBufferData[currentVertexIndex].m_coords = QVector3D(x, y, z) + trans;
		}
	}

	// indexes are genererated row-by-row
	// The unrolled grid looks like this, with the first vertex colum repeated to show the wrap-around
	// 8   9  10  11   8
	// 4   5   6   7   4
	// 0   1   2   3   0
	//
	// The triangles in the bottom-most row are then: (0 4 1) (1 4 5) (1 5 2) ... (3 7 0) (0 7 4)
	for (unsigned int i=0; i<nSeg; ++i) {
		unsigned int topCircleVertexStart = (i+1)*nSeg2;  // i = 0 -> 4
		unsigned int bottomCircleVertexStart = i*nSeg2;   // i = 0 -> 0

		// we add always 2 triangles
		for (unsigned int j=0; j<nSeg2; ++j, currentElementIndex += 6) {
			// add 0 4 1
			indexBufferData[currentElementIndex  ] = vertexStart + bottomCircleVertexStart + j;
			indexBufferData[currentElementIndex+1] = vertexStart + topCircleVertexStart + j;
			indexBufferData[currentElementIndex+2] = vertexStart + bottomCircleVertexStart + (j+1) % nSeg2;
			// add 1 4 5
			indexBufferData[currentElementIndex+4] = vertexStart + bottomCircleVertexStart + (j+1) % nSeg2;
			indexBufferData[currentElementIndex+3] = vertexStart + topCircleVertexStart + j;
			indexBufferData[currentElementIndex+5] = vertexStart + topCircleVertexStart + (j+1) % nSeg2;
		}
	}
}


void updateSphereColors(const QColor & c, unsigned int & currentVertexIndex, std::vector<ColorRGBA> & colorBufferData) {
	unsigned int nSeg = SPHERE_SEGMENTS; // number of segments to split 180° into
	unsigned int nSeg2 = nSeg*2; // number of segments to split 360° into

	// the vertex 0 is (1, 0, 0)*radius
	colorBufferData[currentVertexIndex] = c;
	++currentVertexIndex;

	// now generate the vertexes
	for (unsigned int i=1; i<nSeg; ++i) {

		for (unsigned int j=0; j<nSeg2; ++j, ++currentVertexIndex) {
			colorBufferData[currentVertexIndex] = c;
		}
	}

	// now add last vertex
	// the vertex 1 + nSeg*nSeg2 is (-1, 0, 0)*radius
	colorBufferData[currentVertexIndex] = c;
	++currentVertexIndex;
}


// ** Ikosaeder **

void addIkosaeder(const IBKMK::Vector3D & p, const std::vector<QColor> & cols, double radius,
				  unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
				  std::vector<VertexCR> & vertexBufferData, std::vector<GLuint> & indexBufferData)
{
	QVector3D trans = IBKVector2QVector(p);

	unsigned int nVertices = 12;
	vertexBufferData.resize(vertexBufferData.size() + nVertices);
	std::vector<unsigned int> v0{0,5,1,9,8,3,6,2,10,11,0,5};
	std::vector<unsigned int> v1{11,4,5,4,9,4,3,4,2,4,11};
	std::vector<unsigned int>v2{6,7,8,7,1,7,0,7,10,7,6};
	indexBufferData.resize(indexBufferData.size() + v0.size()+v1.size()+v2.size()+3);

	//add vertices
	const double phi = 0.5 * (1+std::sqrt(5));

	radius *= 0.5;
	vertexBufferData[currentVertexIndex+0  ].m_coords = QVector3D(-1.0, phi, 0.0)*radius + trans;	//0
	vertexBufferData[currentVertexIndex+1  ].m_coords = QVector3D( 1.0, phi, 0.0)*radius + trans;	//1
	vertexBufferData[currentVertexIndex+2  ].m_coords = QVector3D(-1.0, -phi, 0.0)*radius + trans;	//2
	vertexBufferData[currentVertexIndex+3  ].m_coords = QVector3D( 1.0, -phi, 0.0)*radius + trans;	//3
	vertexBufferData[currentVertexIndex+4  ].m_coords = QVector3D(0.0, -1.0, phi)*radius + trans;	//4
	vertexBufferData[currentVertexIndex+5  ].m_coords = QVector3D(0.0,  1.0, phi)*radius + trans;	//5
	vertexBufferData[currentVertexIndex+6  ].m_coords = QVector3D(0.0, -1.0, -phi)*radius + trans;	//6
	vertexBufferData[currentVertexIndex+7  ].m_coords = QVector3D(0.0,  1.0, -phi)*radius + trans;	//7
	vertexBufferData[currentVertexIndex+8  ].m_coords = QVector3D(phi, 0.0, -1.0)*radius + trans;	//8
	vertexBufferData[currentVertexIndex+9  ].m_coords = QVector3D(phi, 0.0,  1.0)*radius + trans;	//9
	vertexBufferData[currentVertexIndex+10 ].m_coords = QVector3D(-phi, 0.0, -1.0)*radius + trans;//10
	vertexBufferData[currentVertexIndex+11 ].m_coords = QVector3D(-phi, 0.0,  1.0)*radius + trans;//11

	for (unsigned int i=0; i<12; ++i)
	vertexBufferData[currentVertexIndex+i  ].m_colors = QtExt::QVector3DFromQColor(cols[i]);

	for (unsigned int i=0; i<v0.size(); ++i)
		indexBufferData[currentElementIndex+i] = currentVertexIndex+ v0[i];
	indexBufferData[currentElementIndex+v0.size()] = VIC3D_STRIP_STOP_INDEX;
	currentElementIndex += v0.size() + 1;

	for (unsigned int i=0; i<v1.size(); ++i)
		indexBufferData[currentElementIndex+i] = currentVertexIndex + v1[i];
	indexBufferData[currentElementIndex+v1.size()] = VIC3D_STRIP_STOP_INDEX;
	currentElementIndex += v1.size() + 1;

	for (unsigned int i=0; i<v2.size(); ++i)
		indexBufferData[currentElementIndex+i] = currentVertexIndex + v2[i];
	indexBufferData[currentElementIndex+v2.size()] = VIC3D_STRIP_STOP_INDEX;
	currentElementIndex += v2.size() + 1;

	currentVertexIndex += nVertices;
}



// *** High-level objects ***

// This adds two planes, one after another, with the second one facing the opposite direction.
void addSurface(const VICUS::Surface & s,
				unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
				std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData,
				std::vector<GLuint> & indexBufferData)
{
	// skip invalid geometry
	if (!s.geometry().isValid())
		return;
	// change color depending on visibility state and selection state
	QColor col = s.m_color;
	// invisible objects are not drawn, and selected objects are drawn by a different object (and are hence invisible in this
	// object as well).
	if (!s.m_visible || s.m_selected) {
		col.setAlphaF(0);
	}
	// first add the plane regular
	addPlane(s.geometry().triangulationData(), col, currentVertexIndex, currentElementIndex, vertexBufferData, colorBufferData, indexBufferData, false);
	// then add the plane again inverted
	addPlane(s.geometry().triangulationData(), col, currentVertexIndex, currentElementIndex, vertexBufferData, colorBufferData, indexBufferData, true);
}


void addSubSurface(const VICUS::Surface & s, unsigned int subSurfaceIndex,
				unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
				std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData,
				std::vector<GLuint> & indexBufferData)
{
	// skip invalid geometry
	if (!s.geometry().isValid())
		return;
	const VICUS::PlaneTriangulationData & subTriangulation = s.geometry().holeTriangulationData()[subSurfaceIndex];
	const VICUS::SubSurface & sub = s.subSurfaces()[subSurfaceIndex];
	// change color depending on visibility state and selection state
	QColor col = sub.m_color;
	// invisible objects are not drawn, and selected objects are drawn by a different object (and are hence invisible in this
	// object as well).
	if (!sub.m_visible || sub.m_selected) {
		col.setAlphaF(0);
	}
	// first add the plane regular
	addPlane(subTriangulation, col, currentVertexIndex, currentElementIndex, vertexBufferData, colorBufferData, indexBufferData, false);
	// then add the plane again inverted
	addPlane(subTriangulation, col, currentVertexIndex, currentElementIndex, vertexBufferData, colorBufferData, indexBufferData, true);
}


// This updates the surface color of the two planes generated from a single surface definition
void updateColors(const VICUS::Surface & s, unsigned int & currentVertexIndex, std::vector<ColorRGBA> & colorBufferData) {
	// skip invalid geometry
	if (!s.geometry().isValid())
		return;
	// change color depending on visibility and selection state
	// invisible objects are not drawn, and selected objects are drawn by a different object (and are hence invisible in this
	// object as well).
	QColor col = s.m_color;
	if (!s.m_visible || s.m_selected) {
		col.setAlphaF(0);
	}
	// call updatePlaneColor() twice, since we have front and backside planes to color
	updateColors(s.geometry().triangulationData(), col, currentVertexIndex, colorBufferData);
	updateColors(s.geometry().triangulationData(), col, currentVertexIndex, colorBufferData);
}


void addBox(const std::vector<IBKMK::Vector3D> & v, const QColor & c,
			unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
			std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData, std::vector<GLuint> & indexBufferData)
{
	FUNCID(addBox);
	// Box geometry is given by 0,1,2,3 and 4,5,6,7 vertexes

	// as long as this is not performance-critical, we use the convenient way and just reuse VICUS::PlaneGeometry

	// floor and ceiling polygons are always planes
	{
		IBKMK::Polygon3D p(VICUS::Polygon2D::T_Rectangle, v[0], v[3], v[1]);
		if (!p.isValid())
			throw IBK::Exception("Invalid polygon.", FUNC_ID);
		VICUS::PlaneGeometry g1(p);
		addPlane(g1.triangulationData(), c, currentVertexIndex, currentElementIndex, vertexBufferData, colorBufferData, indexBufferData, false);
	}
	{
		IBKMK::Polygon3D p(VICUS::Polygon2D::T_Rectangle, v[4], v[5], v[7]);
		VICUS::PlaneGeometry g1(p);
		addPlane(g1.triangulationData(), c, currentVertexIndex, currentElementIndex, vertexBufferData, colorBufferData, indexBufferData, false);
	}

	// the sides are not necessarily planes
#if 1
	for (unsigned int i=0; i<3; ++i) {
		addPlane(VICUS::PlaneTriangulationData(v[(0+i)%8], v[(1+i)%8], v[(5+i)%8]), c,
			currentVertexIndex, currentElementIndex, vertexBufferData, colorBufferData, indexBufferData, false);
		addPlane(VICUS::PlaneTriangulationData(v[(0+i)%8], v[(5+i)%8], v[(4+i)%8]), c,
			currentVertexIndex, currentElementIndex, vertexBufferData, colorBufferData, indexBufferData, false);
	}
	// last plane is special
	addPlane(VICUS::PlaneTriangulationData(v[3], v[0], v[4]), c,
		currentVertexIndex, currentElementIndex, vertexBufferData, colorBufferData, indexBufferData, false);
	addPlane(VICUS::PlaneTriangulationData(v[3], v[4], v[7]), c,
		currentVertexIndex, currentElementIndex, vertexBufferData, colorBufferData, indexBufferData, false);

#else

	{
		VICUS::PlaneGeometry g1(VICUS::Polygon3D::T_Rectangle, v[0], v[1], v[4]);
		addPlane(g1.triangulationData(), c, currentVertexIndex, currentElementIndex, vertexBufferData, colorBufferData, indexBufferData, false);
	}
	{
		VICUS::PlaneGeometry g1(VICUS::Polygon3D::T_Rectangle, v[1], v[2], v[5]);
		addPlane(g1.triangulationData(), c, currentVertexIndex, currentElementIndex, vertexBufferData, colorBufferData, indexBufferData, false);
	}
	{
		VICUS::PlaneGeometry g1(VICUS::Polygon3D::T_Rectangle, v[2], v[3], v[6]);
		addPlane(g1.triangulationData(), c, currentVertexIndex, currentElementIndex, vertexBufferData, colorBufferData, indexBufferData, false);
	}
	{
		VICUS::PlaneGeometry g1(VICUS::Polygon3D::T_Rectangle, v[3], v[0], v[7]);
		addPlane(g1.triangulationData(), c, currentVertexIndex, currentElementIndex, vertexBufferData, colorBufferData, indexBufferData, false);
	}
#endif
}






} // namespace Vic3D
