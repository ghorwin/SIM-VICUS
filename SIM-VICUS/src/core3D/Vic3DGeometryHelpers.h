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
void addPlane(const VICUS::PlaneTriangulationData & g, const QColor & c,
			  unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
			  std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData,
			  std::vector<GLuint> & indexBufferData,
			  bool inverted);

/*! Adds a plane to a vertex, color and index buffer.
	Index buffer contains data for GL_TRIANGLE_STRIP.
*/
void addPlaneAsStrip(const IBKMK::Vector3D & a, const IBKMK::Vector3D & b, const IBKMK::Vector3D & d, const QColor & c,
			  unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
			  std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData,
			  std::vector<GLuint> & indexBufferData);

/*! Same as addPlane, but only adds coordinates (no normals, no color buffer).
	Index buffer contains data for triangles.
*/
void addPlane(const VICUS::PlaneTriangulationData & g,
			  unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
			  std::vector<VertexC> & vertexBufferData, std::vector<GLuint> & indexBufferData);

/*! This updates the plane color in the vertex buffer. */
void updateColors(const VICUS::PlaneTriangulationData & g, const QColor & c,
				  unsigned int & currentVertexIndex, std::vector<ColorRGBA> & colorBufferData);


// ** Cylinders **

/*! Adds a cylinder mesh to a vertex, color and index buffer. */
void addCylinder(const IBKMK::Vector3D & p1, const IBKMK::Vector3D & p2, const QColor & c, double radius,
				 unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
				 std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData,
				 std::vector<GLuint> & indexBufferData, bool closed = false);

/*! Same as addCylinder, but only adds coordinates (no normals, no color buffer).
	Also, the index buffer contains indexes to be drawn with GL_TRIANGLES, whereas the other addCylinder function
	generates index buffer data for GL_TRIANGLE_STRIP.
*/
void addCylinder(const IBKMK::Vector3D & p1, const IBKMK::Vector3D & p2, double radius,
				 unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
				 std::vector<VertexC> & vertexBufferData, std::vector<GLuint> & indexBufferData);

/*! Adds a cylinder mesh to a vertex, color and index buffer. */
void updateCylinderColors(const QColor & c, unsigned int & currentVertexIndex, std::vector<ColorRGBA> & colorBufferData);

// ** Spheres **

/*! Adds a sphere mesh to a vertex, color and index buffer.
	Index buffer is populated to be drawn with GL_TRIANGLE_STRIP.
*/
void addSphere(const IBKMK::Vector3D & p, const QColor & c, double radius,
			   unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
			   std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData,
			   std::vector<GLuint> & indexBufferData);

/*! Same as addSphere, but only adds coordinates (no normals, no color buffer).
	Also, the index buffer contains indexes to be drawn with GL_TRIANGLES, whereas the other addCylinder function
	generates index buffer data for GL_TRIANGLE_STRIP.
*/
void addSphere(const IBKMK::Vector3D & p, double radius,
			   unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
			   std::vector<VertexC> & vertexBufferData, std::vector<GLuint> & indexBufferData);

/*! This updates the surface color of the selected surface in the color buffer. */
void updateSphereColors(const QColor & c, unsigned int & currentVertexIndex, std::vector<ColorRGBA> & colorBufferData);

// ** Ikosaeder **

/*! Adds an ikosaeder with individual colors a vertex+color buffer (no normals). */
void addIkosaeder(const IBKMK::Vector3D & p, const std::vector<QColor> & cols, double radius,
				 unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
				 std::vector<VertexCR> & vertexBufferData, std::vector<GLuint> & indexBufferData);




// *** High-level data structures ***
//
// Note: these functions use the functions above.

/*! This function adds a surface object to a vertex, color and index buffer.
	Index buffer contains data for triangles. Contains code to adjust color based on visibility and selection settings.
*/
void addSurface(const VICUS::Surface & s,
				unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
				std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData,
				std::vector<GLuint> & indexBufferData);

/*! This function adds a sub-surface object to a vertex, color and index buffer.
	Index buffer contains data for triangles. Contains code to adjust color based on visibility and selection settings.
*/
void addSubSurface(const VICUS::Surface & s, unsigned int subSurfaceIndex,
				unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
				std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData,
				std::vector<GLuint> & indexBufferData);

/*! This updates the surface color of the selected surface in the color buffer. */
void updateColors(const VICUS::Surface & s, unsigned int & currentVertexIndex,
				  std::vector<ColorRGBA> & colorBufferData);




/*! This updates the surface color of the selected surface in the color buffer. */
void updateColors(const VICUS::NetworkNode & n,
				  unsigned int & currentVertexIndex, std::vector<ColorRGBA> & colorBufferData);


} // namespace Vic3D


#endif // VIC3DGEOMETRYHELPERS_H
