/************************************************************************************

OpenGL with Qt - Tutorial
-------------------------
Autor      : Andreas Nicolai <andreas.nicolai@gmx.net>
Repository : https://github.com/ghorwin/OpenGLWithQt-Tutorial
License    : BSD License,
			 see https://github.com/ghorwin/OpenGLWithQt-Tutorial/blob/master/LICENSE

************************************************************************************/

#ifndef Vic3DNewPolygonObjectH
#define Vic3DNewPolygonObjectH

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include "Vic3DVertex.h"

#include <VICUS_PlaneGeometry.h>

QT_BEGIN_NAMESPACE
class QOpenGLShaderProgram;
QT_END_NAMESPACE

namespace Vic3D {

class CoordinateSystemObject;

/*! This object is painted when a new polygon is being drawn.
	It paints a red line from the last confirmed polygon vertex to the current
	coordinate's position (accessing translation coordinates from CoordinateSystemObject directly).
	And if more than 3 vertices have been places already, a transparent plane is painted.

	Uses the same shader as the coordinate system object, but is drawn with blending enabled.
*/
class NewPolygonObject {
public:
	NewPolygonObject();

	/*! The function is called during OpenGL initialization, where the OpenGL context is current.
		This only initializes the buffers and vertex array object, but does not allocate data.
		This is done in a call to updateBuffers();
	*/
	void create(QOpenGLShaderProgram * shaderProgramm, const CoordinateSystemObject * coordSystemObject);
	void destroy();

	/*! Populates the color and vertex buffer with data for the "last segment" line and the polygon.
		Resizes vertex and element buffers on GPU memory and copies data from locally stored vertex/element arrays to GPU.

		Basically transfers data in m_vertexBufferData, m_colorBufferData and m_elementBufferData to GPU memory.
	*/
	void updateBuffers();

	/*! Binds the vertex array object and renders the geometry. */
	void render();


	/*! Cached pointer to coordinate system object - used to retrieve current's 3D cursor position. */
	const CoordinateSystemObject	*m_coordSystemObject = nullptr;

	/*! Stores the current geometry of the painted polygon. */
	VICUS::PlaneGeometry			m_planeGeometry;
	unsigned int					m_firstLineVertex = 0;

	/*! Vertex buffer in CPU memory, holds data of all vertices (coords and normals). */
	std::vector<Vertex>				m_vertexBufferData;
	/*! Color buffer in CPU memory, holds colors of all vertices (same size as m_vertexBufferData). */
	std::vector<ColorRGBA>			m_colorBufferData;
	/*! Index buffer on CPU memory. */
	std::vector<GLshort>			m_indexBufferData;

	/*! VertexArrayObject, references the vertex, color and index buffers. */
	QOpenGLVertexArrayObject		m_vao;

	/*! Handle for vertex buffer on GPU memory. */
	QOpenGLBuffer					m_vertexBufferObject;
	/*! Handle for color buffer on GPU memory. */
	QOpenGLBuffer					m_colorBufferObject;
	/*! Handle for index buffer on GPU memory */
	QOpenGLBuffer					m_indexBufferObject;
};

} // namespace Vic3D

#endif // Vic3DNewPolygonObjectH
