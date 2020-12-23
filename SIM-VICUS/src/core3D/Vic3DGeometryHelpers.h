#ifndef VIC3DGEOMETRYHELPERS_H
#define VIC3DGEOMETRYHELPERS_H

#include <VICUS_Surface.h>
#include <VICUS_NetworkEdge.h>
#include <vector>
#include "Vic3DVertex.h"
#include <qopengl.h>

namespace Vic3D {


// *** Primitives ***


// ** Planes (PlaneGeometry/Polygons) **

/*! Adds a plane to a vertex, color and index buffer.
	Index buffer contains data for triangles.
	The 'inverted' flag indicates, that a plane should be added with reversed
	indexes (normal vector facing the other direction).
*/
void addPlane(const VICUS::PlaneGeometry & g, const QColor & c,
			  unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
			  std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData,
			  std::vector<GLshort> & indexBufferData,
			  bool inverted);

/*! Same as addPlane, but only adds coordinates (no normals, no color buffer).
	Index buffer contains data for triangles.
*/
void addPlane(const VICUS::PlaneGeometry & g,
			  unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
			  std::vector<VertexC> & vertexBufferData, std::vector<GLshort> & indexBufferData);

/*! This updates the plane color in the vertex buffer. */
void updateColors(const VICUS::PlaneGeometry & g, const QColor & c,
				  unsigned int & currentVertexIndex, std::vector<ColorRGBA> & colorBufferData);


// ** Cylinders **

/*! Adds a cylinder mesh to a vertex, color and index buffer. */
void addCylinder(const IBKMK::Vector3D & p1, const IBKMK::Vector3D & p2, const QColor & c, double radius,
				 unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
				 std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData,
				 std::vector<GLshort> & indexBufferData);

/*! Adds a cylinder mesh to a vertex, color and index buffer. */
void updateCylinderColors(const QColor & c, unsigned int & currentVertexIndex, std::vector<ColorRGBA> & colorBufferData);

// ** Spheres **

/*! Adds a sphere mesh to a vertex, color and index buffer. */
void addSphere(const IBKMK::Vector3D & p, const QColor & c, double radius,
			   unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
			   std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData,
			   std::vector<GLshort> & indexBufferData);

/*! This updates the surface color of the selected surface in the color buffer. */
void updateSphereColors(const QColor & c, unsigned int & currentVertexIndex, std::vector<ColorRGBA> & colorBufferData);

// ** Ikosaeder **

/*! Adds an ikosaeder with individual colors a vertex+color buffer (no normals). */
void addIkosaeder(const IBKMK::Vector3D & p, const std::vector<QColor> & cols, double radius,
				 unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
				 std::vector<VertexCR> & vertexBufferData, std::vector<GLushort> & indexBufferData);




// *** High-level data structures ***
//
// Note: these functions use the functions above.

/*! This function adds a surface object to a vertex, color and index buffer.
	Index buffer contains data for triangles. Contains code to adjust color based on visibility and selection settings.
*/
void addSurface(const VICUS::Surface & s,
				unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
				std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData,
				std::vector<GLshort> & indexBufferData);

/*! This updates the surface color of the selected surface in the color buffer. */
void updateColors(const VICUS::Surface & s, unsigned int & currentVertexIndex,
				  std::vector<ColorRGBA> & colorBufferData);




/*! This updates the surface color of the selected surface in the color buffer. */
void updateColors(const VICUS::NetworkNode & n,
				  unsigned int & currentVertexIndex, std::vector<ColorRGBA> & colorBufferData);


} // namespace Vic3D


#endif // VIC3DGEOMETRYHELPERS_H
