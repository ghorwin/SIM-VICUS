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

/*! A container for all the boxes.
	Basically creates the geometry of the individual boxes and populates the buffers.
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
		Use the function updateColorBuffer() to only update the color buffer.
	*/
	void updateBuffers();
	/*! Resizes color buffer on GPU memory and copies data from locally stored color data array to GPU. 	*/
	void updateColorBuffer();

	/*! Binds the vertex array object and renders the geometry. */
	void render();

	std::vector<Vertex>			m_vertexBufferData;
	/*! Index buffer on CPU memory. */
	std::vector<GLshort>		m_elementBufferData;

	/*! Wraps an OpenGL VertexArrayObject, that references the vertex, color and index buffers. */
	QOpenGLVertexArrayObject	m_vao;

	/*! Vertex buffer on GPU memory. */
	QOpenGLBuffer				m_vbo;
	/*! Color buffer on GPU memory. */
	QOpenGLBuffer				m_vboColors;
	/*! Index buffer on GPU memory */
	QOpenGLBuffer				m_ebo;
};

} // namespace Vic3D

#endif // Vic3DOpaqueGeometryObjectH
