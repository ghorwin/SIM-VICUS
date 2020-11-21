#include "Vic3DGeometryHelpers.h"

#include <VICUS_Conversions.h>
#include <VICUS_NetworkLine.h>

#include <QQuaternion>

namespace Vic3D {

void addSurface(const VICUS::Surface & s,
				unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
				std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData, std::vector<GLshort> & indexBufferData)
{
	// skip invalid geometry
	if (!s.m_geometry.isValid())
		return;
	addPlane(s.m_geometry, s.m_color, currentVertexIndex, currentElementIndex, vertexBufferData, colorBufferData, indexBufferData);
}


void addPlane(const VICUS::PlaneGeometry & g, const QColor & col,
			  unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
			  std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData, std::vector<GLshort> & indexBufferData)
{
	// different handling based on surface type
	switch (g.m_type) {
		case VICUS::PlaneGeometry::T_Triangle : {
			// insert 3 vertexes
			vertexBufferData.resize(vertexBufferData.size()+3);
			colorBufferData.resize(colorBufferData.size()+3);
			// 4 elements (3 for the triangle, 1 primitive restart index)
			indexBufferData.resize(indexBufferData.size()+4);

			vertexBufferData[currentVertexIndex    ].m_coords = VICUS::IBKVector2QVector(g.m_vertexes[0]);
			vertexBufferData[currentVertexIndex + 1].m_coords = VICUS::IBKVector2QVector(g.m_vertexes[1]);
			vertexBufferData[currentVertexIndex + 2].m_coords = VICUS::IBKVector2QVector(g.m_vertexes[2]);
			QVector3D n = VICUS::IBKVector2QVector(g.m_normal);
			vertexBufferData[currentVertexIndex    ].m_normal = n;
			vertexBufferData[currentVertexIndex + 1].m_normal = n;
			vertexBufferData[currentVertexIndex + 2].m_normal = n;

			colorBufferData[currentVertexIndex     ] = col;
			colorBufferData[currentVertexIndex  + 1] = col;
			colorBufferData[currentVertexIndex  + 2] = col;

			// anti-clock-wise winding order for all triangles in strip
			indexBufferData[currentElementIndex    ] = currentVertexIndex + 0;
			indexBufferData[currentElementIndex + 1] = currentVertexIndex + 1;
			indexBufferData[currentElementIndex + 2] = currentVertexIndex + 2;
			indexBufferData[currentElementIndex + 3] = 0xFFFF; // set stop index


			// finally advance buffer indexes
			currentVertexIndex += 3;
			currentElementIndex += 4;
		} break;

		case VICUS::PlaneGeometry::T_Rectangle : {
			// insert 4 vertexes
			vertexBufferData.resize(vertexBufferData.size()+4);
			colorBufferData.resize(colorBufferData.size()+4);
			// 5 elements (4 for the two triangles in a strip, 1 primitive restart index)
			indexBufferData.resize(indexBufferData.size()+5);

			// 4 indexes anti-clock wise in vertex memory
			QVector3D a = VICUS::IBKVector2QVector(g.m_vertexes[0]);
			QVector3D b = VICUS::IBKVector2QVector(g.m_vertexes[1]);
			QVector3D d = VICUS::IBKVector2QVector(g.m_vertexes[2]);
			vertexBufferData[currentVertexIndex    ].m_coords = a;
			vertexBufferData[currentVertexIndex + 1].m_coords = b;
			QVector3D c = b + (d - a);
			vertexBufferData[currentVertexIndex + 2].m_coords = c;
			vertexBufferData[currentVertexIndex + 3].m_coords = d;

			QVector3D n = VICUS::IBKVector2QVector(g.m_normal);
			vertexBufferData[currentVertexIndex    ].m_normal = n;
			vertexBufferData[currentVertexIndex + 1].m_normal = n;
			vertexBufferData[currentVertexIndex + 2].m_normal = n;
			vertexBufferData[currentVertexIndex + 3].m_normal = n;

			colorBufferData[currentVertexIndex     ] = col;
			colorBufferData[currentVertexIndex  + 1] = col;
			colorBufferData[currentVertexIndex  + 2] = col;
			colorBufferData[currentVertexIndex  + 3] = col;

			// anti-clock-wise winding order for all triangles in strip
			//
			// options are:
			// a) 3, 0, 2, 1
			// b) 0, 1, 3, 2
			indexBufferData[currentElementIndex    ] = currentVertexIndex + 0;
			indexBufferData[currentElementIndex + 1] = currentVertexIndex + 1;
			indexBufferData[currentElementIndex + 2] = currentVertexIndex + 3;
			indexBufferData[currentElementIndex + 3] = currentVertexIndex + 2;
			indexBufferData[currentElementIndex + 4] = 0xFFFF; // set stop index

			// finally advance buffer indexes
			currentVertexIndex += 4;
			currentElementIndex += 5;
		} break;

		case VICUS::PlaneGeometry::T_Polygon : {
			// insert as many vertexes as there are in the polygon
			unsigned int nvert = g.m_vertexes.size();
			vertexBufferData.resize(vertexBufferData.size()+nvert);
			colorBufferData.resize(colorBufferData.size()+nvert);
			// nvert indexes + primitive restart index
			indexBufferData.resize(indexBufferData.size()+nvert+1);

			QVector3D n = VICUS::IBKVector2QVector(g.m_normal);

			// add all vertices to buffer
			for (unsigned int i=0; i<nvert; ++i) {
				// add vertex and
				unsigned int vIdx = currentVertexIndex + i;
				vertexBufferData[vIdx].m_coords = VICUS::IBKVector2QVector(g.m_vertexes[i]);
				vertexBufferData[vIdx].m_normal = n;
				colorBufferData[vIdx] = col;
				// build up triangle strip index buffer
				bool odd = (i % 2 != 0);
				// odd vertices are added anti-clock-wise from start, even vertices are added clock-wise from end
				unsigned int j = i / 2;
				if (odd)
					indexBufferData[currentElementIndex + i] = currentVertexIndex + j;
				else
					indexBufferData[currentElementIndex + i] = currentVertexIndex + nvert - j - 1;
			}

			indexBufferData[currentElementIndex + nvert] = 0xFFFF; // set stop index

			// finally advance buffer indexes
			currentVertexIndex += nvert;
			currentElementIndex += nvert + 1;
		} break;
	} // switch
}


void addNetworkEdge(const VICUS::NetworkEdge & p, unsigned int & currentVertexIndex, unsigned int & currentElementIndex, std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData, std::vector<GLshort> & indexBufferData) {
	VICUS::NetworkLine l(p);
	IBKMK::Vector3D p1(l.m_x1, l.m_y1, 0);
	IBKMK::Vector3D p2(l.m_x2, l.m_y2, 0);
	addCylinder(p1, p2, Qt::red, 0.1, currentVertexIndex, currentElementIndex, vertexBufferData, colorBufferData, indexBufferData);
}


void addCylinder(const IBKMK::Vector3D & p1, const IBKMK::Vector3D & p2, const QColor & c, double radius,
			  unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
				 std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData, std::vector<GLshort> & indexBufferData)
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


void addSphere(const IBKMK::Vector3D & p, const QColor & c, double radius,
			   unsigned int & currentVertexIndex,
			   unsigned int & currentElementIndex,
			   std::vector<Vertex> & vertexBufferData,
			   std::vector<ColorRGBA> & colorBufferData,
			   std::vector<GLshort> & indexBufferData)
{

	QVector3D trans = VICUS::IBKVector2QVector(p);

#if 0
	unsigned int nVertices = 12;
	vertexBufferData.resize(vertexBufferData.size() + nVertices);
	colorBufferData.resize(colorBufferData.size() + nVertices);
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

	vertexBufferData[currentVertexIndex+0  ].m_normal = QVector3D(-1.0, phi, 0.0).normalized();	//0
	vertexBufferData[currentVertexIndex+1  ].m_normal = QVector3D( 1.0, phi, 0.0).normalized();	//1
	vertexBufferData[currentVertexIndex+2  ].m_normal = QVector3D(-1.0, -phi, 0.0).normalized();	//2
	vertexBufferData[currentVertexIndex+3  ].m_normal = QVector3D( 1.0, -phi, 0.0).normalized();	//3
	vertexBufferData[currentVertexIndex+4  ].m_normal = QVector3D(0.0, -1.0, phi).normalized();	//4
	vertexBufferData[currentVertexIndex+5  ].m_normal = QVector3D(0.0,  1.0, phi).normalized();	//5
	vertexBufferData[currentVertexIndex+6  ].m_normal = QVector3D(0.0, -1.0, -phi).normalized();	//6
	vertexBufferData[currentVertexIndex+7  ].m_normal = QVector3D(0.0,  1.0, -phi).normalized();	//7
	vertexBufferData[currentVertexIndex+8  ].m_normal = QVector3D(phi, 0.0, -1.0).normalized();	//8
	vertexBufferData[currentVertexIndex+9  ].m_normal = QVector3D(phi, 0.0,  1.0).normalized();	//9
	vertexBufferData[currentVertexIndex+10 ].m_normal = QVector3D(-phi, 0.0, -1.0).normalized();//10
	vertexBufferData[currentVertexIndex+11 ].m_normal = QVector3D(-phi, 0.0,  1.0).normalized();//11


	for (unsigned int i=0; i<nVertices;++i)
		colorBufferData[currentVertexIndex + i] = c;
	colorBufferData[currentVertexIndex + 4 ] = c.darker(); // QColor(Qt::green);
	colorBufferData[currentVertexIndex + 7 ] = c.lighter(); // QColor(Qt::green);


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
//	currentElementIndex += v0.size()+v1.size()+v2.size()+3;
#else

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

#endif
}

} // namespace Vic3D
