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

class SVPropVertexListWidget;

namespace Vic3D {

class CoordinateSystemObject;
class ShaderProgram;

/*! This object is painted when a new polygon is being drawn.
	It paints a red line from the last confirmed polygon vertex to the current
	coordinate's position (accessing translation coordinates from CoordinateSystemObject directly).
	And if more than 3 vertices have been places already, a transparent plane is painted.

	Uses the same shader as the coordinate system object, but is drawn with blending enabled.
*/
class NewPolygonObject {
public:

	/*! Defines the state that the system is currently at, when adding new geometry.
		The state is initialized when some "Add..." button is pressed and afterwards
		changed through placement of vertexes or others.
	*/
	enum NewGeometryMode {
		/*! A rectangle is being created - after the second vertex has been placed,
			the third vertex is a projection onto the line perpendicular to the first and
			in the plane of the three vertexes. Also, a fourth vertex is automatically added,
			that closes the rectangle. A rectangle plane is drawn (using Vic3DNewRectObject) once the first
			two vertexes have been placed. Confirming the third point, the plane is created.
		*/
		NGM_Rect,
		/*! A polygon is being created - after the third non-collinear vertex has been drawn, all subsequent
			points are projected into the created plane. The polygon is shown using Vic3DNewPolygonObject
			To create the polygon, it must be valid (not winding).
		*/
		NGM_Polygon,
		NGM_ZoneFloor,
		NGM_ZoneExtrusion,
		NUM_NGM
	};


	NewPolygonObject();

	/*! The function is called during OpenGL initialization, where the OpenGL context is current.
		This only initializes the buffers and vertex array object, but does not allocate data.
		This is done in a call to updateBuffers();
	*/
	void create(ShaderProgram * shaderProgram, const CoordinateSystemObject * coordSystemObject);
	void destroy();

	/*! Appends a vertex to the plane geometry and updates the draw buffer. */
	void appendVertex(const IBKMK::Vector3D & p);

	/*! Removes the vertex at the given position.
		idx must be less than number of vertexes!
	*/
	void removeVertex(unsigned int idx);

	/*! Clear current geometry (clears also all buffers). */
	void clear();

	/*! Calls on_pushButtonFinish_clicked() in VertexListWidget and adds the polygon to the data structure. */
	void finish();

	/*! This function is to be called whenever the movable coordinate system changes its (snapped) position.
		The function first compares the point with the currently set point - if no change is recognized, nothing happens.
		If the point was indeed moved, the GPU buffer will be updated.

		The vertex passed is the position of the local coordinate system. This may be, however, outside the
		plane (if already enough vertices have been placed to form a plane). If this is the case, the function
		computes the projection onto the plane and adds the projected point coordinates instead.

		Actually what happens depends on the m_newGeometryMode.
	*/
	void newLocalCoordinateSystemPosition(const QVector3D & p);


	/*! Renders opaque parts of geometry. */
	void renderOpqaue();
	/*! Renders transparent parts of geometry. */
	void renderTransparent();

	/*! Returns true, if enough vertexes have been collected to complete the geometry.
		This depends on the type of geometry being generated.
	*/
	bool canComplete() const { return m_planeGeometry.isValid(); }

	/*! Can be used to check if object has data to paint - this can be used to check if there is a polygon object at all.
		Hence, this function could be named 'isVisible()' as well.
	*/
	bool hasData() const { return m_planeGeometry.isValid(); }

	/*! Provides read-only access to the current plane geometry. */
	const VICUS::PlaneGeometry planeGeometry() const { return m_planeGeometry; }

	/*! Cached pointer to vertex list widget - for direct communication, when a node has been placed.
		The function appendVertex() relays this call to vertex list widget.
		Pointer is set (by SVPropertyWidget) once widget has been created.
	*/
	SVPropVertexListWidget			*m_vertexListWidget = nullptr;

private:
	/*! Populates the color and vertex buffer with data for the "last segment" line and the polygon.
		Resizes vertex and element buffers on GPU memory and copies data from locally stored vertex/element arrays to GPU.

		Basically transfers data in m_vertexBufferData, m_colorBufferData and m_elementBufferData to GPU memory.
	*/
	void updateBuffers();


	/*! Shader program (not owned). */
	ShaderProgram					*m_shaderProgram = nullptr;

	/*! Cached pointer to coordinate system object - used to retrieve current's 3D cursor position (not owned). */
	const CoordinateSystemObject	*m_coordSystemObject = nullptr;

	/*! Stores the current geometry of the painted polygon. */
	VICUS::PlaneGeometry			m_planeGeometry;

	/*! This list holds all points a the drawing method.
		This list must not be a valid polygon
	*/
	std::vector<IBKMK::Vector3D>	m_vertexList;

	/*! Vertex buffer in CPU memory, holds data of all vertices (coords).
		The last vertex is always the vertex of the current movable coordinate system's location.
		The line will be drawn between the last and the one before last vertex, using array draw command.
	*/
	std::vector<VertexC>			m_vertexBufferData;
	/*! Index buffer on CPU memory (only for the triangle strip). */
	std::vector<GLuint>				m_indexBufferData;

	/*! VertexArrayObject, references the vertex, color and index buffers. */
	QOpenGLVertexArrayObject		m_vao;

	/*! Handle for vertex buffer on GPU memory. */
	QOpenGLBuffer					m_vertexBufferObject;
	/*! Handle for index buffer on GPU memory */
	QOpenGLBuffer					m_indexBufferObject;

	/*! Defines the current geometry mode that we are in.
		This determines the visualization of the current object and what happens if a vertex is placed
		(i.e. user clicks somewhere in the scene) and local coordinate system is moved.
	*/
	NewGeometryMode					m_newGeometryMode;

};

} // namespace Vic3D

#endif // Vic3DNewPolygonObjectH
