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

/*! A container for geometry to be rendered with triangle strips.
	This object is used multiple times and is not associated with any particular
	part of the scene or building.
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
	void render();

	/*! Vertex buffer in CPU memory, holds data of all vertices (coords and normals). */
	std::vector<Vertex>			m_vertexBufferData;
	/*! Color buffer in CPU memory, holds colors of all vertices (same size as m_vertexBufferData). */
	std::vector<ColorRGBA>		m_colorBufferData;
	/*! Index buffer on CPU memory. */
	std::vector<GLshort>		m_indexBufferData;

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
