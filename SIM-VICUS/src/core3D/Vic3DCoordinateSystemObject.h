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

/*! Draws the local coordinate system with adjustable ball/icosaeder coordinate system indicator.
	The coordinate system object is composed of several objects, and only some of them are shown
	together.

	Regular coordinate system object:
	- one big sphere in the middle          indexBufferObject 0
	- one opaque cylinder per axis          indexBufferObject 1-3
	- one small sphere at each axis end     indexBufferObject 4-6

	Coordinate system while "positioning coordinate system":
	- colored lines for each coordinate system axis meeting in the center (to allow very precise snapping)
	- semi-transparent sphere in the middle indexBufferObject 7
*/
class CoordinateSystemObject {
public:

	CoordinateSystemObject();

	/*! The function is called during OpenGL initialization, where the OpenGL context is current. */
	void create(ShaderProgram * shaderProgram);
	void destroy();

	/*! Binds the buffer and paints. */
	void renderOpaque();
	/*! Binds the buffer and paints. */
	void renderTransparent();

	/*! Modifies the location of the local coordinate system.
		This function also tells the edit geometry widget its new position.
	*/
	void setTranslation(const QVector3D & translation);

	/*! Returns the current translation of the coordinate system. */
	const QVector3D & translation() const { return m_transform.translation(); }

	void setRotation(const QQuaternion & rotMatrix);

	/*! Returns current transformation matrix (origin and rotation). */
	void setTransform(const Transform3D & transform);
	/*! Sets new transformation matrix. */
	Transform3D transform() const { return m_transform; }

	/*! Returns the inverse transformation matrix. */
	QMatrix4x4 inverseTransformationMatrix() const { return m_inverseMatrix; }

	/*! Returns the local X-coordinate axis. */
	QVector3D localXAxis() const { return m_transform.rotation().rotatedVector(QVector3D(1,0,0)); }
	/*! Returns the local Y-coordinate axis. */
	QVector3D localYAxis() const { return m_transform.rotation().rotatedVector(QVector3D(0,1,0)); }
	/*! Returns the local Z-coordinate axis. */
	QVector3D localZAxis() const { return m_transform.rotation().rotatedVector(QVector3D(0,0,1)); }

private:
	/*! Updates the inverse matrix. */
	void updateInverse();

	/*! The transformation object, transforms the coordinate system to its position and orientation in
		the scene.
	*/
	Transform3D					m_transform;

	/*! Inverse transformation, needed to fix light/view position for phong shader. */
	QMatrix4x4					m_inverseMatrix;

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

	/*! VertexArrayObject for lines. */
	QOpenGLVertexArrayObject	m_lineVao;
	/*! Holds positions of lines (x-x, y-y, z-z lines, 6 VertexC coordinates). */
	QOpenGLBuffer				m_lineVbo;


	/*! Vertex buffer in CPU memory, holds data of all vertices (coords and normals). */
	std::vector<Vertex>			m_vertexBufferData;
	/*! Color buffer in CPU memory, holds colors of all vertices (same size as m_vertexBufferData). */
	std::vector<ColorRGBA>		m_colorBufferData;
	/*! Index buffer on CPU memory. */
	std::vector<GLuint>			m_indexBufferData;

	/*! Index of vertexes for axis lines. For each axis line we have two indexes, so
		the x-axis is drawn with vertexes starting at m_axisLinesVertexIndex, the y-axis starting at
		m_axisLinesVertexIndex+2 etc.
	*/
	unsigned int				m_axisLinesVertexIndex;
};

} // namespace Vic3D


#endif // Vic3DCoordinateSystemObjectH
