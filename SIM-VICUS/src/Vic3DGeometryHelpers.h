#ifndef VIC3DGEOMETRYHELPERS_H
#define VIC3DGEOMETRYHELPERS_H

#include <VICUS_Surface.h>
#include <VICUS_NetworkEdge.h>
#include <vector>
#include "Vic3DVertex.h"
#include <qopengl.h>

namespace Vic3D {

/*! This function adds a surface object to a vertex, color and index buffer. */
void addSurface(const VICUS::Surface & s,
				unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
				std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData, std::vector<GLshort> & indexBufferData);

/*! Adds a plane to a vertex, color and index buffer. */
void addPlane(const VICUS::PlaneGeometry & g, const QColor & c,
			  unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
			  std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData, std::vector<GLshort> & indexBufferData);

/*! This takes a network edge and visualizes it in the scene as a cylinder.
	The diameter is currently taken as fixed value (later a configurable parameter).
*/
void addNetworkEdge(const VICUS::NetworkEdge & p,
					unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
					std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData, std::vector<GLshort> & indexBufferData);


/*! Adds a cylinder mesh to a vertex, color and index buffer. */
void addCylinder(const IBKMK::Vector3D & p1, const IBKMK::Vector3D & p2, const QColor & c, double radius,
			  unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
			  std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData, std::vector<GLshort> & indexBufferData);

} // namespace Vic3D


#endif // VIC3DGEOMETRYHELPERS_H
