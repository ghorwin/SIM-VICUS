#include "Vic3DGeometryHelpers.h"

#include <VICUS_Conversions.h>
#include <VICUS_NetworkLine.h>

#include <QQuaternion>

#include "SVSettings.h"

namespace Vic3D {


// *** Primitives ***



void addPlane(const VICUS::PlaneGeometry & g, const QColor & col,
			  unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
			  std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData, std::vector<GLshort> & indexBufferData,
			  bool inverted)
{
	// add vertex data to buffers
	unsigned int nVertexes = g.vertexes().size();
	// insert count vertexes
	vertexBufferData.resize(vertexBufferData.size()+nVertexes);
	colorBufferData.resize(colorBufferData.size()+nVertexes);
	// set data
	QVector3D n = VICUS::IBKVector2QVector(g.normal());
	if (inverted)
		n *= -1;
	for (unsigned int i=0; i<nVertexes; ++i) {
		vertexBufferData[currentVertexIndex + i].m_coords = VICUS::IBKVector2QVector(g.vertexes()[i]);
		vertexBufferData[currentVertexIndex + i].m_normal = n;
		colorBufferData[currentVertexIndex  + i] = col;
	}

	// index buffer is populated differently, depending on geometry type
	switch (g.type()) {
		case VICUS::PlaneGeometry::T_Triangle : {
			// 3 elements for the triangle
			indexBufferData.resize(indexBufferData.size()+3);
			// anti-clock-wise winding order for all triangles in strip
			if (inverted) {
				indexBufferData[currentElementIndex    ] = currentVertexIndex + 0;
				indexBufferData[currentElementIndex + 1] = currentVertexIndex + 2;
				indexBufferData[currentElementIndex + 2] = currentVertexIndex + 1;
			}
			else {
				indexBufferData[currentElementIndex    ] = currentVertexIndex + 0;
				indexBufferData[currentElementIndex + 1] = currentVertexIndex + 1;
				indexBufferData[currentElementIndex + 2] = currentVertexIndex + 2;
			}
			// advance index in element/index buffer
			currentElementIndex += 3;
		} break;

		case VICUS::PlaneGeometry::T_Rectangle : {
			// 6 elements (2 triangles)
			indexBufferData.resize(indexBufferData.size()+6);

			if (inverted) {
				indexBufferData[currentElementIndex    ] = currentVertexIndex + 0;
				indexBufferData[currentElementIndex + 1] = currentVertexIndex + 2;
				indexBufferData[currentElementIndex + 2] = currentVertexIndex + 1;
				indexBufferData[currentElementIndex + 3] = currentVertexIndex + 2;
				indexBufferData[currentElementIndex + 4] = currentVertexIndex + 0;
				indexBufferData[currentElementIndex + 5] = currentVertexIndex + 3;
			}
			else {
				// anti-clock-wise winding order for all triangles in strip
				// 0, 1, 2
				// 2, 3, 0
				indexBufferData[currentElementIndex    ] = currentVertexIndex + 0;
				indexBufferData[currentElementIndex + 1] = currentVertexIndex + 1;
				indexBufferData[currentElementIndex + 2] = currentVertexIndex + 2;
				indexBufferData[currentElementIndex + 3] = currentVertexIndex + 2;
				indexBufferData[currentElementIndex + 4] = currentVertexIndex + 3;
				indexBufferData[currentElementIndex + 5] = currentVertexIndex + 0;
			}

			// advance index in element/index buffer
			currentElementIndex += 6;
		} break;

		case VICUS::PlaneGeometry::T_Polygon : {
			unsigned int triangleIndexCount = g.triangles().size()*3;
			indexBufferData.resize(indexBufferData.size()+triangleIndexCount);
			// add all triangles

			for (const VICUS::PlaneGeometry::triangle_t & t : g.triangles()) {
				if (inverted) {
					indexBufferData[currentElementIndex    ] = currentVertexIndex + t.a;
					indexBufferData[currentElementIndex + 1] = currentVertexIndex + t.c;
					indexBufferData[currentElementIndex + 2] = currentVertexIndex + t.b;
				}
				else {
					indexBufferData[currentElementIndex    ] = currentVertexIndex + t.a;
					indexBufferData[currentElementIndex + 1] = currentVertexIndex + t.b;
					indexBufferData[currentElementIndex + 2] = currentVertexIndex + t.c;
				}
				// advance index in element/index buffer
				currentElementIndex += 3;
			}

			// index in element/index buffer has already been advanced
		} break;

		case VICUS::PlaneGeometry::NUM_T:; // just to make compiler happy
	} // switch

	// finally advance vertex index
	currentVertexIndex += nVertexes;
}


void addPlane(const VICUS::PlaneGeometry & g, unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
			  std::vector<VertexC> & vertexBufferData, std::vector<GLshort> & indexBufferData)
{
	// add vertex data to buffers
	unsigned int nVertexes = g.vertexes().size();
	// insert count vertexes
	vertexBufferData.resize(vertexBufferData.size()+nVertexes);
	// set data
	for (unsigned int i=0; i<nVertexes; ++i)
		vertexBufferData[currentVertexIndex + i].m_coords = VICUS::IBKVector2QVector(g.vertexes()[i]);

	// index buffer is populated differently, depending on geometry type
	switch (g.type()) {
		case VICUS::PlaneGeometry::T_Triangle : {
			// 3 elements for the triangle
			indexBufferData.resize(indexBufferData.size()+3);
			// anti-clock-wise winding order for all triangles in strip
			indexBufferData[currentElementIndex    ] = currentVertexIndex + 0;
			indexBufferData[currentElementIndex + 1] = currentVertexIndex + 1;
			indexBufferData[currentElementIndex + 2] = currentVertexIndex + 2;
			// advance index in element/index buffer
			currentElementIndex += 3;
		} break;

		case VICUS::PlaneGeometry::T_Rectangle : {
			// 6 elements (2 triangles)
			indexBufferData.resize(indexBufferData.size()+6);

			// anti-clock-wise winding order for all triangles in strip
			// 0, 1, 2
			// 2, 3, 0
			indexBufferData[currentElementIndex    ] = currentVertexIndex + 0;
			indexBufferData[currentElementIndex + 1] = currentVertexIndex + 1;
			indexBufferData[currentElementIndex + 2] = currentVertexIndex + 2;
			indexBufferData[currentElementIndex + 3] = currentVertexIndex + 2;
			indexBufferData[currentElementIndex + 4] = currentVertexIndex + 3;
			indexBufferData[currentElementIndex + 5] = currentVertexIndex + 0;

			// advance index in element/index buffer
			currentElementIndex += 6;
		} break;

		case VICUS::PlaneGeometry::T_Polygon : {
			unsigned int triangleIndexCount = g.triangles().size()*3;
			indexBufferData.resize(indexBufferData.size()+triangleIndexCount);
			// add all triangles

			for (const VICUS::PlaneGeometry::triangle_t & t : g.triangles()) {
				indexBufferData[currentElementIndex    ] = currentVertexIndex + t.a;
				indexBufferData[currentElementIndex + 1] = currentVertexIndex + t.b;
				indexBufferData[currentElementIndex + 2] = currentVertexIndex + t.c;
				// advance index in element/index buffer
				currentElementIndex += 3;
			}

			// index in element/index buffer has already been advanced
		} break;

		case VICUS::PlaneGeometry::NUM_T:; // just to make compiler happy
	} // switch

	// finally advance vertex index
	currentVertexIndex += nVertexes;
}


void updateColors(const VICUS::PlaneGeometry & g, const QColor & col, unsigned int & currentVertexIndex, std::vector<ColorRGBA> & colorBufferData) {
	// different handling based on surface type
	switch (g.type()) {
		case VICUS::PlaneGeometry::T_Triangle : {
			colorBufferData[currentVertexIndex     ] = col;
			colorBufferData[currentVertexIndex  + 1] = col;
			colorBufferData[currentVertexIndex  + 2] = col;

			// finally advance buffer indexes
			currentVertexIndex += 3;
		} break;

		case VICUS::PlaneGeometry::T_Rectangle : {
			colorBufferData[currentVertexIndex     ] = col;
			colorBufferData[currentVertexIndex  + 1] = col;
			colorBufferData[currentVertexIndex  + 2] = col;
			colorBufferData[currentVertexIndex  + 3] = col;
			currentVertexIndex += 4;
		} break;

		case VICUS::PlaneGeometry::T_Polygon : {
			unsigned int nvert = g.vertexes().size();
			// add all vertices to buffer
			for (unsigned int i=0; i<nvert; ++i)
				colorBufferData[currentVertexIndex + i] = col;
			// finally advance buffer indexes
			currentVertexIndex += nvert;
		} break;

		case VICUS::PlaneGeometry::NUM_T:; // just to make compiler happy
	} // switch
}


void addCylinder(const IBKMK::Vector3D & p1, const IBKMK::Vector3D & p2, const QColor & c, double radius,
				 unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
				 std::vector<Vertex> & vertexBufferData,
				 std::vector<ColorRGBA> & colorBufferData,
				 std::vector<GLshort> & indexBufferData)
{
	// we generate vertices for a cylinder starting at 0,0,0 and extending to 1,0,0 (x-axis is the rotation axis)

	// after each generated vertex, it is scaled, rotated into position, and translated to p1

#define PI_CONST 3.14159265

	// our vertices are numbered 0, 1, 2 with odd vertices at x=0, and even vertices at x=1

	IBKMK::Vector3D cylinderAxis = p2-p1;
	double L = cylinderAxis.magnitude();

	// this is the rotation matrix to be applied to each generated vertex and normal vector
	QQuaternion rot = QQuaternion::rotationTo(QVector3D(1,0,0), VICUS::IBKVector2QVector(cylinderAxis));
	QVector3D trans = VICUS::IBKVector2QVector(p1);

	// add the first two vertices (which are also the last)

	unsigned int nSeg = 16; // number of segments to generate
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
	for (unsigned int i=0; i<nSeg*2; ++i, ++currentElementIndex)
		indexBufferData[currentElementIndex] = i + vertexIndexStart;

	// finally add first two vertices again
	indexBufferData[currentElementIndex++] = vertexIndexStart;
	indexBufferData[currentElementIndex++] = vertexIndexStart+1;
	indexBufferData[currentElementIndex++] = 0xFFFF; // set stop index
}


void addCylinder(const IBKMK::Vector3D & p1, const IBKMK::Vector3D & p2, double radius,
				 unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
				 std::vector<VertexC> & vertexBufferData, std::vector<GLshort> & indexBufferData)
{
	// we generate vertices for a cylinder starting at 0,0,0 and extending to 1,0,0 (x-axis is the rotation axis)

	// after each generated vertex, it is scaled, rotated into position, and translated to p1

#define PI_CONST 3.14159265

	// our vertices are numbered 0, 1, 2 with odd vertices at x=0, and even vertices at x=1

	IBKMK::Vector3D cylinderAxis = p2-p1;
	double L = cylinderAxis.magnitude();

	// this is the rotation matrix to be applied to each generated vertex and normal vector
	QQuaternion rot = QQuaternion::rotationTo(QVector3D(1,0,0), VICUS::IBKVector2QVector(cylinderAxis));
	QVector3D trans = VICUS::IBKVector2QVector(p1);

	// add the first two vertices (which are also the last)

	unsigned int nSeg = 16; // number of segments to generate
	// insert nSeg*2 vertexes
	vertexBufferData.resize(vertexBufferData.size() + nSeg*2);
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
	}

	// now add elements
	for (unsigned int i=0; i<nSeg*2; ++i, ++currentElementIndex)
		indexBufferData[currentElementIndex] = i + vertexIndexStart;

	// finally add first two vertices again
	indexBufferData[currentElementIndex++] = vertexIndexStart;
	indexBufferData[currentElementIndex++] = vertexIndexStart+1;
	indexBufferData[currentElementIndex++] = 0xFFFF; // set stop index
}


void updateCylinderColors(const QColor & c, unsigned int & currentVertexIndex, std::vector<ColorRGBA> & colorBufferData) {
	unsigned int nSeg = 16; // number of segments to generate

	// insert vertexes, 2 per segment
	for (unsigned int i=0; i<nSeg; ++i, currentVertexIndex += 2) {
		// set colors
		colorBufferData[currentVertexIndex     ] = c;
		colorBufferData[currentVertexIndex  + 1] = c;
	}
}


void addSphere(const IBKMK::Vector3D & p, const QColor & c, double radius,
			   unsigned int & currentVertexIndex,
			   unsigned int & currentElementIndex,
			   std::vector<Vertex> & vertexBufferData,
			   std::vector<ColorRGBA> & colorBufferData,
			   std::vector<GLshort> & indexBufferData)
{

	QVector3D trans = VICUS::IBKVector2QVector(p);

	unsigned int nSeg = 8; // number of segments to split 180° into
	unsigned int nSeg2 = nSeg*2; // number of segments to split 360° into

	vertexBufferData.resize(vertexBufferData.size() + (nSeg-1)*nSeg2 + 2);
	colorBufferData.resize(colorBufferData.size() + (nSeg-1)*nSeg2 + 2);
	// (nSeg+1)*2 + 1 element indexes ((nSeg+1)*2 for the triangle strip, 1 primitive restart index)
	indexBufferData.resize(indexBufferData.size() + nSeg2*2 + 2 + 1 /* stop index */  +  (nSeg-2)*(nSeg2*2 + 2 + 1 /* stop index */ )  + nSeg2*2 + 1 + 1 /* stop index */ );

	unsigned int vertexStart = currentVertexIndex;

	// the vertex 0 is (1, 0, 0)*radius
	vertexBufferData[currentVertexIndex].m_coords = QVector3D(radius, 0.0, 0.0) + trans;
	vertexBufferData[currentVertexIndex].m_normal = QVector3D(1, 0.0, 0.0);
	colorBufferData[currentVertexIndex] = c;
	++currentVertexIndex;

	// now generate the vertexes
	for (unsigned int i=1; i<nSeg; ++i) {
		double beta = PI_CONST*i/nSeg;
		double flat_radius = std::sin(beta)*radius;
		double nx = std::cos(beta);
		double x = nx*radius;

		for (unsigned int j=0; j<nSeg2; ++j, ++currentVertexIndex) {
			double angle = -2*PI_CONST*j/nSeg2;
			double ny = std::cos(angle);
			double y = ny*flat_radius;
			double nz = std::sin(angle);
			double z = nz*flat_radius;

			vertexBufferData[currentVertexIndex].m_coords = QVector3D(x, y, z) + trans;
			vertexBufferData[currentVertexIndex].m_normal = QVector3D(nx, ny, nz);
			colorBufferData[currentVertexIndex] = c;
		}
	}

	// now add last vertex
	// the vertex 1 + nSeg*nSeg2 is (-1, 0, 0)*radius
	vertexBufferData[currentVertexIndex].m_coords = QVector3D(-radius, 0.0, 0.0) + trans;
	vertexBufferData[currentVertexIndex].m_normal = QVector3D(-1, 0.0, 0.0);
	colorBufferData[currentVertexIndex] = c;
	++currentVertexIndex;


	// first circle
	unsigned int lastVertex = vertexStart+1; // start with first vertex in for circle
	for (unsigned int i=0; i<nSeg2*2; ++i, ++currentElementIndex) {
		if (i % 2 == 0)
			indexBufferData[currentElementIndex] = lastVertex++;
		else {
			indexBufferData[currentElementIndex] = vertexStart;
		}
	}
	indexBufferData[currentElementIndex++] = vertexStart+1; // finish circle with first vertex
	indexBufferData[currentElementIndex++] = 0xFFFF; // set stop index

	// middle circles
	for (unsigned int j=1; j<nSeg-1; ++j) {
		vertexStart = lastVertex;
		for (unsigned int i=0; i<nSeg2; ++i, currentElementIndex += 2, ++lastVertex) {
			indexBufferData[currentElementIndex  ] = lastVertex;
			indexBufferData[currentElementIndex+1] = lastVertex - nSeg2;
		}
		indexBufferData[currentElementIndex++] = vertexStart;
		indexBufferData[currentElementIndex++] = vertexStart - nSeg2;

		indexBufferData[currentElementIndex++] = 0xFFFF; // set stop index
	}

	vertexStart = lastVertex;
	for (unsigned int i=0; i<nSeg2*2; ++i, ++currentElementIndex) {
		if (i % 2 == 0)
			indexBufferData[currentElementIndex] = currentVertexIndex-1;
		else {
			indexBufferData[currentElementIndex] = lastVertex++ - nSeg2;
		}
	}
	indexBufferData[currentElementIndex++] = currentVertexIndex-1;
	indexBufferData[currentElementIndex++] = vertexStart - nSeg2;
	indexBufferData[currentElementIndex++] = 0xFFFF; // set stop index

}


void addSphere(const IBKMK::Vector3D & p, double radius,
			   unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
			   std::vector<VertexC> & vertexBufferData, std::vector<GLshort> & indexBufferData)
{

	QVector3D trans = VICUS::IBKVector2QVector(p);

	unsigned int nSeg = 8; // number of segments to split 180° into
	unsigned int nSeg2 = nSeg*2; // number of segments to split 360° into

	vertexBufferData.resize(vertexBufferData.size() + (nSeg-1)*nSeg2 + 2);
	// (nSeg+1)*2 + 1 element indexes ((nSeg+1)*2 for the triangle strip, 1 primitive restart index)
	indexBufferData.resize(indexBufferData.size() + nSeg2*2 + 2 + 1 /* stop index */  +  (nSeg-2)*(nSeg2*2 + 2 + 1 /* stop index */ )  + nSeg2*2 + 1 + 1 /* stop index */ );

	unsigned int vertexStart = currentVertexIndex;

	// the vertex 0 is (1, 0, 0)*radius
	vertexBufferData[currentVertexIndex].m_coords = QVector3D(radius, 0.0, 0.0) + trans;
	++currentVertexIndex;

	// now generate the vertexes
	for (unsigned int i=1; i<nSeg; ++i) {
		double beta = PI_CONST*i/nSeg;
		double flat_radius = std::sin(beta)*radius;
		double nx = std::cos(beta);
		double x = nx*radius;

		for (unsigned int j=0; j<nSeg2; ++j, ++currentVertexIndex) {
			double angle = -2*PI_CONST*j/nSeg2;
			double ny = std::cos(angle);
			double y = ny*flat_radius;
			double nz = std::sin(angle);
			double z = nz*flat_radius;

			vertexBufferData[currentVertexIndex].m_coords = QVector3D(x, y, z) + trans;
		}
	}

	// now add last vertex
	// the vertex 1 + nSeg*nSeg2 is (-1, 0, 0)*radius
	vertexBufferData[currentVertexIndex].m_coords = QVector3D(-radius, 0.0, 0.0) + trans;
	++currentVertexIndex;


	// first circle
	unsigned int lastVertex = vertexStart+1; // start with first vertex in for circle
	for (unsigned int i=0; i<nSeg2*2; ++i, ++currentElementIndex) {
		if (i % 2 == 0)
			indexBufferData[currentElementIndex] = lastVertex++;
		else {
			indexBufferData[currentElementIndex] = vertexStart;
		}
	}
	indexBufferData[currentElementIndex++] = vertexStart+1; // finish circle with first vertex
	indexBufferData[currentElementIndex++] = 0xFFFF; // set stop index

	// middle circles
	for (unsigned int j=1; j<nSeg-1; ++j) {
		vertexStart = lastVertex;
		for (unsigned int i=0; i<nSeg2; ++i, currentElementIndex += 2, ++lastVertex) {
			indexBufferData[currentElementIndex  ] = lastVertex;
			indexBufferData[currentElementIndex+1] = lastVertex - nSeg2;
		}
		indexBufferData[currentElementIndex++] = vertexStart;
		indexBufferData[currentElementIndex++] = vertexStart - nSeg2;

		indexBufferData[currentElementIndex++] = 0xFFFF; // set stop index
	}

	vertexStart = lastVertex;
	for (unsigned int i=0; i<nSeg2*2; ++i, ++currentElementIndex) {
		if (i % 2 == 0)
			indexBufferData[currentElementIndex] = currentVertexIndex-1;
		else {
			indexBufferData[currentElementIndex] = lastVertex++ - nSeg2;
		}
	}
	indexBufferData[currentElementIndex++] = currentVertexIndex-1;
	indexBufferData[currentElementIndex++] = vertexStart - nSeg2;
	indexBufferData[currentElementIndex++] = 0xFFFF; // set stop index
}


void updateSphereColors(const QColor & c, unsigned int & currentVertexIndex, std::vector<ColorRGBA> & colorBufferData) {
	unsigned int nSeg = 8; // number of segments to split 180° into
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


void addIkosaeder(const IBKMK::Vector3D & p, const std::vector<QColor> & cols, double radius,
				  unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
				  std::vector<VertexCR> & vertexBufferData, std::vector<GLushort> & indexBufferData)
{
	QVector3D trans = VICUS::IBKVector2QVector(p);

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
	vertexBufferData[currentVertexIndex+i  ].m_colors = VICUS::QVector3DFromQColor(cols[i]);

	for (unsigned int i=0; i<v0.size(); ++i)
		indexBufferData[currentElementIndex+i] = currentVertexIndex+ v0[i];
	indexBufferData[currentElementIndex+v0.size()] = 0xFFFF;
	currentElementIndex += v0.size() + 1;

	for (unsigned int i=0; i<v1.size(); ++i)
		indexBufferData[currentElementIndex+i] = currentVertexIndex + v1[i];
	indexBufferData[currentElementIndex+v1.size()] = 0xFFFF;
	currentElementIndex += v1.size() + 1;

	for (unsigned int i=0; i<v2.size(); ++i)
		indexBufferData[currentElementIndex+i] = currentVertexIndex + v2[i];
	indexBufferData[currentElementIndex+v2.size()] = 0xFFFF;
	currentElementIndex += v2.size() + 1;

	currentVertexIndex += nVertices;
}



// *** High-level objects ***

// This adds two planes, one after another, with the second one facing the opposite direction.
void addSurface(const VICUS::Surface & s,
				unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
				std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData, std::vector<GLshort> & indexBufferData)
{
	// skip invalid geometry
	if (!s.m_geometry.isValid())
		return;
	// change color depending on visibility state and selection state
	QColor col = s.m_color;
	if (!s.m_visible)
		col.setAlphaF(0);
	else if (s.m_selected) {
		col = SVSettings::instance().m_themeSettings[SVSettings::instance().m_theme].m_selectedSurfaceColor;
	}
	// first add the plane regular
	addPlane(s.m_geometry, col, currentVertexIndex, currentElementIndex, vertexBufferData, colorBufferData, indexBufferData, false);
	// then add the plane again inverted
	addPlane(s.m_geometry, col, currentVertexIndex, currentElementIndex, vertexBufferData, colorBufferData, indexBufferData, true);
}


// This updates the surface color of the two planes generated from a single surface definition
void updateColors(const VICUS::Surface & s, unsigned int & currentVertexIndex, std::vector<ColorRGBA> & colorBufferData) {
	// skip invalid geometry
	if (!s.m_geometry.isValid())
		return;
	// change color depending on visibility and selection state
	// invisible objects are now drawn, and selected objects are drawn by a different object (and are hence invisible in this
	// object as well).
	QColor col = s.m_color;
	if (!s.m_visible || s.m_selected) {
		col.setAlphaF(0);
	}
	// call updatePlaneColor() twice, since we have front and backside planes to color
	updateColors(s.m_geometry, col, currentVertexIndex, colorBufferData);
	updateColors(s.m_geometry, col, currentVertexIndex, colorBufferData);
}









} // namespace Vic3D
