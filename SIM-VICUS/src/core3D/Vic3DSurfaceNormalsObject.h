/************************************************************************************

OpenGL with Qt - Tutorial
-------------------------
Autor      : Andreas Nicolai <andreas.nicolai@gmx.net>
Repository : https://github.com/ghorwin/OpenGLWithQt-Tutorial
License    : BSD License,
			 see https://github.com/ghorwin/OpenGLWithQt-Tutorial/blob/master/LICENSE

************************************************************************************/

#ifndef Vic3DSurfaceNormalsObjectH
#define Vic3DSurfaceNormalsObjectH

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <QVector3D>

QT_BEGIN_NAMESPACE
class QOpenGLShaderProgram;
QT_END_NAMESPACE

namespace Vic3D {

class ShaderProgram;

/*! This object draws surface normal vectors as lines with uniform size and color.
	The object uses the simple color shader.
*/
class SurfaceNormalsObject {
public:
	/*! The function is called during OpenGL initialization, where the OpenGL context is current. */
	void create(ShaderProgram * shaderProgram);
	void destroy();

	/*! This updates the vertex buffers for all lines based on the current visible geometry. */
	void updateVertexBuffers();

	/*! Binds the buffer and paints. */
	void render();

	/*! Shader program, that the grid is painted with. */
	ShaderProgram				*m_shaderProgram = nullptr;

	/*! Holds the number of vertices (2 for each line), updated in create(), used in render().
		If zero, grid is disabled.
	*/
	GLsizei						m_vertexCount;

	/*! Wraps an OpenGL VertexArrayObject, that references the vertex coordinates. */
	QOpenGLVertexArrayObject	m_vao;
	/*! Holds positions of grid lines. */
	QOpenGLBuffer				m_vbo;
};

} // namespace Vic3D


#endif // Vic3DSurfaceNormalsObjectH
