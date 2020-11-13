/************************************************************************************

OpenGL with Qt - Tutorial
-------------------------
Autor      : Andreas Nicolai <andreas.nicolai@gmx.net>
Repository : https://github.com/ghorwin/OpenGLWithQt-Tutorial
License    : BSD License,
			 see https://github.com/ghorwin/OpenGLWithQt-Tutorial/blob/master/LICENSE

************************************************************************************/

#ifndef OPENGLGRIDOBJECT_H
#define OPENGLGRIDOBJECT_H

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <QVector3D>

QT_BEGIN_NAMESPACE
class QOpenGLShaderProgram;
QT_END_NAMESPACE

namespace Vic3D {

class ShaderProgram;

/*! This class holds all data needed to draw a grid on the screen.
	We have only a coordinate buffer, which holds tightly packed 2 * vec2 (start and end points of lines, xz coords)
	with y=0 implied.
	Grid colors are uniforms, as is background color.

	The grid is drawn with the grid shader program, which is passed to the create() function.

	The grid lines for major and minor grid are always generated, but minor
*/
class GridObject {
public:
	/*! The function is called during OpenGL initialization, where the OpenGL context is current. */
	void create(ShaderProgram * shaderProgram);
	void destroy();

	/*! Binds the buffer and paints. */
	void render();

	/*! Shader program, that the grid is painted with. */
	ShaderProgram				*m_gridShader = nullptr;

	/*! Color for major grid lines. */
	QVector3D					m_majorGridColor;
	/*! Color for minor grid lines. */
	QVector3D					m_minorGridColor;

	/*! Holds the current size of the buffer, updated in create(), used in render().
		If zero, grid is disabled.
	*/
	GLsizei						m_bufferSize;
	/*! Index, where minor grid starts. If same as m_bufferSize, minor grid is disabled. */
	GLsizei						m_minorGridBufferStart;

	/*! Wraps an OpenGL VertexArrayObject, that references the vertex coordinates. */
	QOpenGLVertexArrayObject	m_vao;
	/*! Holds positions of grid lines. */
	QOpenGLBuffer				m_vbo;

	/*! Cached grid width. */
	float						m_width		= 999;
	/*! Cached grid spacing. */
	float						m_spacing	= 999;
};

} // namespace Vic3D


#endif // OPENGLGRIDOBJECT_H
