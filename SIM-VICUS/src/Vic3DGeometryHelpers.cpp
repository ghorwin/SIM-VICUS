#include "Vic3DGeometryHelpers.h"

#include <VICUS_Conversions.h>
#include <VICUS_NetworkLine.h>


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
	addCylinder(p1, p2, Qt::red, currentVertexIndex, currentElementIndex, vertexBufferData, colorBufferData, indexBufferData);
}


void addCylinder(const IBKMK::Vector3D & p1, const IBKMK::Vector3D & p2, const QColor & c,
			  unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
				 std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData, std::vector<GLshort> & indexBufferData)
{
	// we generate vertices for a cylinder starting at 0,0,0 and extending to 1,0,0 (x-axis is the rotation axis)

	// after each generated vertex, it is scaled, rotated into position, and translated to p1

	unsigned int numFaces = 4; // number of faces to generated
#define PI_CONST 3.14159265

	// our vertices are numbered 0, 1, 2 with odd vertices at x=0, and even vertices at x=1

	// add the first two vertices (which are also the last)


	for (unsigned int i=0; i<=numFaces) {
		double angle = PI_CONST*i;
		// for each face we add
	}


}

} // namespace Vic3D
