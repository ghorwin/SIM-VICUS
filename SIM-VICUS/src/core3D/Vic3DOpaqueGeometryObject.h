/************************************************************************************

OpenGL with Qt - Tutorial
-------------------------
Autor      : Andreas Nicolai <andreas.nicolai@gmx.net>
Repository : https://github.com/ghorwin/OpenGLWithQt-Tutorial
License    : BSD License,
			 see https://github.com/ghorwin/OpenGLWithQt-Tutorial/blob/master/LICENSE

************************************************************************************/

#ifndef Vic3DOpaqueGeometryObjectH
#define Vic3DOpaqueGeometryObjectH

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include "Vic3DVertex.h"

QT_BEGIN_NAMESPACE
class QOpenGLShaderProgram;
QT_END_NAMESPACE

namespace Vic3D {

/*! A container for geometry to be rendered with either triangle strips or triangles.
	This object is used multiple times and is not associated with any particular
	part of the scene or building. Hence, it is populated with data, externally.
*/
class OpaqueGeometryObject {
public:
	OpaqueGeometryObject();

	/*! The function is called during OpenGL initialization, where the OpenGL context is current.
		This only initializes the buffers and vertex array object, but does not allocate data.
		This is done in a call to updateBuffers();
	*/
	void create(QOpenGLShaderProgram * shaderProgramm);
	void destroy();

	/*! Resizes vertex and element buffers on GPU memory and copies data from locally stored vertex/element arrays to GPU.
		This might be a lengthy operation, so call this as infrequently as possible.

		Basically transfers data in m_vertexBufferData, m_colorBufferData and m_elementBufferData to GPU memory.
		Calls the function updateColorBuffer() internally to update the color buffer.
	*/
	void updateBuffers();
	/*! Only copies the color buffer m_colorBufferData to GPU memory.
		Call this function instead of updateBuffers(), if only colors of objects/visibility have changed.
	*/
	void updateColorBuffer();

	/*! Binds the vertex array object and renders the geometry. */
	void renderOpaque();

	/*! Returns true, if transparent geometry is visible. */
	bool canDrawTransparent() const { return m_transparentStartIndex != (GLsizei)m_indexBufferData.size(); }
	/*! Renders in transparent mode (OpenGL context is expected to be setup correctly, already). */
	void renderTransparent();

	/*! If true, this object expects index buffer to hold indexes suitable for drawing GL_TRIANGLE_STRIP (including
		primitive restart indexes) and uses this to draw the objects.
		If false, index buffer is expected to hold indexes of triangles that are drawn with GL_TRIANGLES (no primitive
		restart index). For 2 triangles (i.e. one rectangle) this means: 4 indexes + 1 = 5 for GL_TRIANGLE_STRIP and
		2*3 = 6 for GL_TRIANGLES.
	*/
	bool						m_drawTriangleStrips = true;

	/*! Vertex buffer in CPU memory, holds data of all vertices (coords and normals). */
	std::vector<Vertex>			m_vertexBufferData;
	/*! Color buffer in CPU memory, holds colors of all vertices (same size as m_vertexBufferData). */
	std::vector<ColorRGBA>		m_colorBufferData;
	/*! Index buffer on CPU memory. */
	std::vector<GLuint>			m_indexBufferData;

	// We have one set of buffers, the vertexBufferObject holds all vertexes of opaque
	// and transparent planes alike. Same applies to the color and index buffer.
	// The variable m_transparentStartIndex holds the index position where the
	// transparent planes start.

	GLsizei						m_transparentStartIndex = 0;

	/*! Maps unique surface/node ID to vertex start index in m_vertexBufferData. */
	std::map<unsigned int, unsigned int>	m_vertexStartMap;

	/*! VertexArrayObject, references the vertex, color and index buffers. */
	QOpenGLVertexArrayObject	m_vao;

	/*! Handle for vertex buffer on GPU memory. */
	QOpenGLBuffer				m_vertexBufferObject;
	/*! Handle for color buffer on GPU memory. */
	QOpenGLBuffer				m_colorBufferObject;
	/*! Handle for index buffer on GPU memory */
	QOpenGLBuffer				m_indexBufferObject;
};

} // namespace Vic3D

#endif // Vic3DOpaqueGeometryObjectH
