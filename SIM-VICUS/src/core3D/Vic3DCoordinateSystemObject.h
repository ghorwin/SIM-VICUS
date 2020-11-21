/************************************************************************************

OpenGL with Qt - Tutorial
-------------------------
Autor      : Andreas Nicolai <andreas.nicolai@gmx.net>
Repository : https://github.com/ghorwin/OpenGLWithQt-Tutorial
License    : BSD License,
			 see https://github.com/ghorwin/OpenGLWithQt-Tutorial/blob/master/LICENSE

************************************************************************************/

#ifndef Vic3DOrbitControllerObjectH
#define Vic3DOrbitControllerObjectH

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <QVector3D>

QT_BEGIN_NAMESPACE
class QOpenGLShaderProgram;
QT_END_NAMESPACE

#include "Vic3DTransform3D.h"

namespace Vic3D {

class ShaderProgram;

/*! Draws the rotation axis for the orbital controller.
	The orbit indicator is a set of line segments, one for the rotation axis, and one for the ring.
*/
class OrbitControllerObject {
public:
	/*! The function is called during OpenGL initialization, where the OpenGL context is current. */
	void create(ShaderProgram * shaderProgram);
	void destroy();

	/*! Binds the buffer and paints. */
	void render();

	/*! The transformation object, transforms the rotation object drawn
		around the z-axis in the coordinate origin, moves it to the center of the orbit
		and rotates it such, that the orbit is correctly drawn.
	*/
	Transform3D					m_transform;

	/*! Shader program. */
	ShaderProgram				*m_shader = nullptr;

	/*! Holds the number of vertices (2 for each line), updated in create(), used in render(). */
	GLsizei						m_vertexCount;

	/*! Wraps an OpenGL VertexArrayObject, that references the vertex coordinates. */
	QOpenGLVertexArrayObject	m_vao;
	/*! Holds coordinates. */
	QOpenGLBuffer				m_vbo;

};

} // namespace Vic3D


#endif // Vic3DOrbitControllerObjectH
