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
*/
class CoordinateSystemObject {
public:

	/*! These enumeration values define which transformation mode is active and how the local coordinate system
		shall be painted.
		Note, some of the transform modes are additive, hence we use a bit mask.

		If all rotation modes are active, this means "passive rotation mode, no rotation axis selected yet".
		Otherwise only one rotation axis can be active and only the orbit for this rotation is shown.
	*/
	enum GeometryTransformMode {
		/*! No transform mode, just a regular coordinate system. */
		TM_None									= 0x00,
		/*! Rotation around X-axis. */
		TM_RotateX								= 0x01,
		/*! Rotation around Y-axis. */
		TM_RotateY								= 0x02,
		/*! Rotation around Z-axis. */
		TM_RotateZ								= 0x04,
		/*! Scale along X-axis. */
		TM_ScaleX								= 0x10,
		/*! Scale along Y-axis. */
		TM_ScaleY								= 0x20,
		/*! Scale along Z-axis. */
		TM_ScaleZ								= 0x40,
		/*! Translation mode (there is no axis-specific handling for translation, since translation locks are
			set separetely)
		*/
		TM_Translate							= 0x100,
	};



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

	/*! A bitmask that indicates the current transform modes for the coordinate system and ultimately
		defines how the local coordinate system shall be drawn. See renderOpaque() for a description of the rules.
	*/
	int m_geometryTransformMode = 0;

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

	std::vector<unsigned int>   m_objectStartIndexes;

	/*! Index of vertexes for axis lines. For each axis line we have two indexes, so
		the x-axis is drawn with vertexes starting at m_axisLinesVertexIndex, the y-axis starting at
		m_axisLinesVertexIndex+2 etc.

		Axis lines are shown when axis-lock is enabled, and when translate/scale operations are in progress
		for a given axis.
	*/
	std::vector<GLint>			m_axisLinesVertexIndex;
};

} // namespace Vic3D


#endif // Vic3DCoordinateSystemObjectH
