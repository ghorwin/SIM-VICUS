/************************************************************************************

OpenGL with Qt - Tutorial
-------------------------
Autor      : Andreas Nicolai <andreas.nicolai@gmx.net>
Repository : https://github.com/ghorwin/OpenGLWithQt-Tutorial
License    : BSD License,
			 see https://github.com/ghorwin/OpenGLWithQt-Tutorial/blob/master/LICENSE

************************************************************************************/

#ifndef Vic3DNewGeometryObjectH
#define Vic3DNewGeometryObjectH

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

/*! This object is painted when new geometry is being created.

	Visualization depends on the geometry being created.

	For polygon drawing:

	It paints a red line from the last confirmed polygon vertex to the current
	coordinate's position.
	And if more than 3 vertices have been places already, a transparent plane is painted.

	For rect drawing:

	The user can click and add two points at will. These must not be the same.
	The third point that is added finishes the shape.
	When two points have been added, the third point will cause the final shape to be shown already.


	Uses two shaders: one for drawing transparent planes in renderTransparent(),
	as the coordinate system object, but is drawn with blending enabled.


	Note: the user may create invalid geometry, or geometry that is simplified during
		  generation of polygons. The list of vertexes actually entered by the user
		  is stored in m_vertexList whereas the generated geometry is stored in the
		  mode-specific storage classes, for example planeGeometry() for polygon mode.
*/
class NewGeometryObject {
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
			points are projected into the created plane. The polygon is shown using Vic3DNewGeometryObject
			To create the polygon, it must be valid (not winding).
		*/
		NGM_Polygon,
		NGM_ZoneFloor,
		NGM_ZoneExtrusion,
		NUM_NGM
	};


	NewGeometryObject();

	/*! The function is called during OpenGL initialization, where the OpenGL context is current.
		This only initializes the buffers and vertex array object, but does not allocate data.
	*/
	void create(ShaderProgram * shaderProgram);

	/*! Destroys allocated resources. */
	void destroy();


	// Functions related to modifying the stored geometry

	/*! This function clears the current buffer and vertex lists. */
	void startNewGeometry(NewGeometryMode m) {
		clear();
		m_newGeometryMode = m;
	}

	/*! Returns the current geometry type. */
	NewGeometryMode newGeometryMode() const {	return m_newGeometryMode; }

	/*! For two-stage geometry construction, you can here switch from one mode to another. */
	void switchTo(NewGeometryMode m);

	/*! Appends a vertex to the geometry.
		This function is used to manually add a vertex, for example when entering a vertex
		by keyboard.
	*/
	void appendVertex(const IBKMK::Vector3D & p);

	/*! Appends a vertex offset.
		This function is used to manually add a vertex as offset to the previously added vertex,
		for example when entering a vertex offset by keyboard.
		If no vertex has been entered before, the offset is taken as first vertex coordinates
		(just like offset from coordinate origin).
	*/
	void appendVertexOffset(const IBKMK::Vector3D & offset);

	/*! Removes the vertex at the given position.
		idx must be less than number of vertexes!
		\note This only works for some new geometry modes.
		This function calls removeVertex() in VertexListPropertyWidget.
	*/
	void removeVertex(unsigned int idx);

	/*! Convenience function for removeVertex() with idx = lastIndex. */
	void removeLastVertex();

	/*! Clear current geometry (clears also all buffers) - nothing is drawn afterwards. */
	void clear();

	/*! Returns true, if enough (valid) data has been collected to complete the geometry.
		This depends on the type of geometry being generated.
	*/
	bool canComplete() const;

	/*! Can be used to check if object has data to paint - this can be used to check if there is a polygon object at all.
		Hence, this function could be named 'isVisible()' as well.
	*/
	bool canDrawTransparent() const;

	/*! Adds the newly created geometry polygon to the data structure. */
	void finish();


	// Functions for retrieving the current geometry/vertex data input

	/*! Provides read-only access to the current plane geometry. */
	const VICUS::PlaneGeometry planeGeometry() const { return m_planeGeometry; }

	/*! Gives access to the internally stored vertex list. */
	const std::vector<IBKMK::Vector3D> & vertexList() const { return m_vertexList; }


	// Other public member functions

	/*! This function is to be called whenever the movable coordinate system changes its (snapped) position.
		The function first compares the point with the currently set point - if no change is recognized, nothing happens.
		If the point was indeed moved, the GPU buffer will be updated.

		The vertex passed is the position of the local coordinate system. This may be, however, outside the
		plane (if already enough vertices have been placed to form a plane). If this is the case, the function
		computes the projection onto the plane and adds the projected point coordinates instead.

		Actually what happens depends on the m_newGeometryMode.
	*/
	void updateLocalCoordinateSystemPosition(const QVector3D & p);



	/*! Renders opaque parts of geometry. */
	void renderOpaque();
	/*! Renders transparent parts of geometry. */
	void renderTransparent();


private:
	/*! Populates the color and vertex buffer with data for the "last segment" line and the polygon.
		Resizes vertex and element buffers on GPU memory and copies data from locally stored vertex/element arrays to GPU.

		Basically transfers data in m_vertexBufferData, m_colorBufferData and m_elementBufferData to GPU memory.

		\param onlyLocalCSMoved Performance enhancement parameter - indicated that only the position
			of the local coordinate system has changed, and thus only a smaller part of the
			buffer may be needed to be redrawn.
	*/
	void updateBuffers(bool onlyLocalCSMoved);


	/*! Shader program (not owned). */
	ShaderProgram					*m_shaderProgram = nullptr;

	/*! Stores the current geometry of the painted polygon or floor polygon. */
	VICUS::PlaneGeometry			m_planeGeometry;

	/*! This list holds all points a the drawing method (even points of collinear segments).
		This list may not give a valid polygon or a polygon at all.
	*/
	std::vector<IBKMK::Vector3D>	m_vertexList;

	/*! Stores the current position of the local coordinate system, which is updated
		(and potentially projected) whenever the local coordinate system moves.
	*/
	QVector3D						m_localCoordinateSystemPosition;

	/*! Defines the current geometry mode that we are in.
		This determines the visualization of the current object and what happens if a vertex is placed
		(i.e. user clicks somewhere in the scene) and local coordinate system is moved.
	*/
	NewGeometryMode					m_newGeometryMode;


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


};

} // namespace Vic3D

#endif // Vic3DNewGeometryObjectH
