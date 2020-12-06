#ifndef VIC3DGEOMETRYHELPERS_H
#define VIC3DGEOMETRYHELPERS_H

#include <VICUS_Surface.h>
#include <VICUS_NetworkEdge.h>
#include <vector>
#include "Vic3DVertex.h"
#include <qopengl.h>

namespace Vic3D {

/*! This function adds a surface object to a vertex, color and index buffer.
	Index buffer contains data for triangles. Contains code to adjust color based on visibility and selection settings.
*/
void addSurface(const VICUS::Surface & s,
				unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
				std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData, std::vector<GLshort> & indexBufferData);

/*! This updates the surface color of the selected surface in the color buffer. */
void updateSurfaceColors(const VICUS::Surface & s, unsigned int & currentVertexIndex, std::vector<ColorRGBA> & colorBufferData);

/*! Adds a plane to a vertex, color and index buffer.
	Index buffer contains data for triangles.
	The 'inverted' flag indicates, that a plane should be added with reversed indexes (normal vector facing the other direction).
*/
void addPlane(const VICUS::PlaneGeometry & g, const QColor & c,
			  unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
			  std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData, std::vector<GLshort> & indexBufferData,
			  bool inverted);

/*! Same as addPlane, but only adds coordinates (no normals, no color buffer).
	Index buffer contains data for triangles.
*/
void addPlane(const VICUS::PlaneGeometry & g,
			  unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
			  std::vector<VertexC> & vertexBufferData, std::vector<GLshort> & indexBufferData);

/*! This updates the plane color in the vertex buffer. */
void updatePlaneColor(const VICUS::PlaneGeometry & g, const QColor & c, unsigned int & currentVertexIndex, std::vector<ColorRGBA> & colorBufferData);


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

/*! Adds a sphere mesh to a vertex, color and index buffer. */
void addSphere(const IBKMK::Vector3D & p, const QColor & c, double radius,
				 unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
				 std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData, std::vector<GLshort> & indexBufferData);

/*! Adds an ikosaeder with individual colors a vertex+color buffer (no normals). */
void addIkosaeder(const IBKMK::Vector3D & p, const std::vector<QColor> & cols, double radius,
				 unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
				 std::vector<VertexCR> & vertexBufferData, std::vector<GLushort> & indexBufferData);

} // namespace Vic3D


#endif // VIC3DGEOMETRYHELPERS_H
