/************************************************************************************

OpenGL with Qt - Tutorial
-------------------------
Autor      : Andreas Nicolai <andreas.nicolai@gmx.net>
Repository : https://github.com/ghorwin/OpenGLWithQt-Tutorial
License    : BSD License,
			 see https://github.com/ghorwin/OpenGLWithQt-Tutorial/blob/master/LICENSE

************************************************************************************/

#ifndef Vic3DCoordinateSystemObjectH
#define Vic3DCoordinateSystemObjectH

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <QVector3D>

QT_BEGIN_NAMESPACE
class QOpenGLShaderProgram;
QT_END_NAMESPACE

#include "Vic3DTransform3D.h"
#include "Vic3DVertex.h"

class SVPropEditGeometry;

namespace Vic3D {

class ShaderProgram;

/*! Draws the coordinate system with adjustable ball/icosaeder coordinate system indicator. */
class CoordinateSystemObject {
public:

	CoordinateSystemObject();

	/*! The function is called during OpenGL initialization, where the OpenGL context is current. */
	void create(ShaderProgram * shaderProgram);
	void destroy();

	/*! Binds the buffer and paints. */
	void render();

	/*! Modifies the location of the local coordinate system.
		This function also tells the edit geometry widget its new position.
	*/
	void setTranslation(const QVector3D & translation);

	/*! Returns the current translation of the coordinate system. */
	const QVector3D & translation() const { return m_transform.translation(); }

	/*! Returns current transformation matrix (origin and rotation). */
	void setTransform(const Transform3D & transform) { m_transform = transform; }
	/*! Sets new transformation matrix. */
	Transform3D transform() const { return m_transform; }

	/*! Cached pointer to geometry edit widget - needed for direct communication. */
	SVPropEditGeometry			*m_propEditGeometry = nullptr;


private:
	/*! The transformation object, transforms the coordinate system to its position and orientation in
		the scene.
	*/
	Transform3D					m_transform;

	/*! Shader program. */
	ShaderProgram				*m_shaderProgram = nullptr;

	/*! Wraps an OpenGL VertexArrayObject, that references the vertex coordinates. */
	QOpenGLVertexArrayObject	m_vao;
	/*! Handle for vertex buffer on GPU memory. */
	QOpenGLBuffer				m_vertexBufferObject;
	/*! Handle for color buffer on GPU memory. */
	QOpenGLBuffer				m_colorBufferObject;
	/*! Handle for index buffer on GPU memory */
	QOpenGLBuffer				m_indexBufferObject;

	/*! Vertex buffer in CPU memory, holds data of all vertices (coords and normals). */
	std::vector<Vertex>			m_vertexBufferData;
	/*! Color buffer in CPU memory, holds colors of all vertices (same size as m_vertexBufferData). */
	std::vector<ColorRGBA>		m_colorBufferData;
	/*! Index buffer on CPU memory. */
	std::vector<GLshort>		m_indexBufferData;


};

} // namespace Vic3D


#endif // Vic3DCoordinateSystemObjectH
