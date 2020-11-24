#include "Vic3DNewPolygonObject.h"

#include <QVector3D>
#include <QOpenGLShaderProgram>
#include <QElapsedTimer>

#include <VICUS_Project.h>
#include <VICUS_Conversions.h>

#include "SVProjectHandler.h"
#include "Vic3DGeometryHelpers.h"
#include "Vic3DCoordinateSystemObject.h"

namespace Vic3D {

NewPolygonObject::NewPolygonObject() :
	m_vertexBufferObject(QOpenGLBuffer::VertexBuffer), // VertexBuffer is the default, so default constructor would have been enough
	m_colorBufferObject(QOpenGLBuffer::VertexBuffer),
	m_indexBufferObject(QOpenGLBuffer::IndexBuffer) // make this an Index Buffer
{
}


void NewPolygonObject::create(QOpenGLShaderProgram * shaderProgramm, const CoordinateSystemObject * coordSystemObject) {
	m_coordSystemObject = coordSystemObject;

	// *** create buffers on GPU memory ***

	// create a new buffer for the vertices and colors, separate buffers because we will modify colors way more often than geometry
	m_vertexBufferObject.create();
	m_vertexBufferObject.setUsagePattern(QOpenGLBuffer::StaticDraw); // usage pattern will be used when tranferring data to GPU

	m_colorBufferObject.create();
	m_colorBufferObject.setUsagePattern(QOpenGLBuffer::StaticDraw);

	// create a new buffer for the indexes
	m_indexBufferObject = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer); // Note: make sure this is an index buffer
	m_indexBufferObject.create();
	m_indexBufferObject.setUsagePattern(QOpenGLBuffer::StaticDraw);


	// *** create and bind Vertex Array Object ***

	// Note: VAO must be bound *before* the element buffer is bound,
	//       because the VAO remembers associated element buffers.
	m_vao.create();
	m_vao.bind(); // now the VAO is active and remembers states modified in following calls

	m_indexBufferObject.bind(); // this registers this index buffer in the currently bound VAO


	// *** set attribute arrays for shader fetch stage ***

#define VERTEX_ARRAY_INDEX 0
#define NORMAL_ARRAY_INDEX 1
#define COLOR_ARRAY_INDEX 2

	m_vertexBufferObject.bind(); // this registers this buffer data object in the currently bound vao; in subsequent
				  // calls to shaderProgramm->setAttributeBuffer() the buffer object is associated with the
				  // respective attribute array that's fed into the shader. When the vao is later bound before
				  // rendering, this association is remembered so that the vertex fetch stage pulls data from
				  // this vbo

	// coordinates
	shaderProgramm->enableAttributeArray(VERTEX_ARRAY_INDEX);
	shaderProgramm->setAttributeBuffer(VERTEX_ARRAY_INDEX, GL_FLOAT, 0, 3 /* vec3 */, sizeof(Vertex));

	// normals
	shaderProgramm->enableAttributeArray(NORMAL_ARRAY_INDEX);
	shaderProgramm->setAttributeBuffer(NORMAL_ARRAY_INDEX, GL_FLOAT, offsetof(Vertex, m_normal), 3 /* vec3 */, sizeof(Vertex));


	m_colorBufferObject.bind(); // now color buffer is active in vao

	// colors
	shaderProgramm->enableAttributeArray(COLOR_ARRAY_INDEX);
	shaderProgramm->setAttributeBuffer(COLOR_ARRAY_INDEX, GL_UNSIGNED_BYTE, 0, 4, 4 /* bytes = sizeof(char) */);

	// Release (unbind) all

	// Mind: you can release the buffer data objects (vbo and vboColors) before or after releasing vao. It does not
	//       matter, because the buffers are associated already with the attribute arrays.
	//       However, YOU MUST NOT release the index buffer (ebo) before releasing the vao, since this would remove
	//       the index buffer association with the vao and when binding the vao before rendering, the element buffer
	//       would not be known and a call to glDrawElements() crashes!
	m_vao.release();

	m_vertexBufferObject.release();
	m_colorBufferObject.release();
	m_indexBufferObject.release();

	m_planeGeometry.m_type = VICUS::PlaneGeometry::T_Polygon;
	// add test data
#if 0
	appendVertex(IBKMK::Vector3D(-5,0,0));
	appendVertex(IBKMK::Vector3D(0,2,0));
	appendVertex(IBKMK::Vector3D(-5,2,0));
	updateBuffers();
#endif
}


void NewPolygonObject::destroy() {
	m_vao.destroy();
	m_vertexBufferObject.destroy();
	m_colorBufferObject.destroy();
	m_indexBufferObject.destroy();
}


void NewPolygonObject::appendVertex(const IBKMK::Vector3D & p) {
	m_planeGeometry.m_vertexes.push_back(p);
	m_planeGeometry.updateNormal();
	updateBuffers();
}


void NewPolygonObject::updateLastVertex(const QVector3D & p) {
	// no vertex added yet? should normally not happen, but during testing we just check it
	if (m_vertexBufferData.empty())
		return;
	// any change to the previously stored point?
	if (p == m_vertexBufferData.back().m_coords)
		return;
	// update last coordinate
	m_vertexBufferData.back().m_coords = p;
	// and update the last part of the buffer (later, for now we just upload the entire buffer again)
	// transfer data stored in m_vertexBufferData
	m_vertexBufferObject.bind();
	m_vertexBufferObject.allocate(m_vertexBufferData.data(), m_vertexBufferData.size()*sizeof(Vertex));
	m_vertexBufferObject.release();
}


void NewPolygonObject::updateBuffers() {
	// create geometry

	// memory layout:
	//   with valid polygon:          vertexBuffer = |polygon_vertexes|coordinate system vertex|
	//   without valid polygon:       vertexBuffer = |last_polygon_vertex|coordinate system vertex|

	// index buffer is only filled if valid polygon exists

	// first copy polygon from PlaneGeometry, if at least 3 vertexes are inserted
	// then add vertex to last

	m_vertexBufferData.clear();
	m_colorBufferData.clear();
	m_indexBufferData.clear();
	unsigned int currentVertexIndex = 0;
	unsigned int currentElementIndex = 0;
	m_firstLineVertex = 0;

	if (m_planeGeometry.isValid()) {
		addPlane(m_planeGeometry, QColor(255,0,128,128), currentVertexIndex, currentElementIndex,
				 m_vertexBufferData, m_colorBufferData, m_indexBufferData);
		// remember index of vertex where "current" line starts
		m_firstLineVertex = currentVertexIndex-1;
	}
	else {
		Q_ASSERT(!m_planeGeometry.m_vertexes.empty());
		m_vertexBufferData.resize(1);
		m_vertexBufferData.back().m_coords = VICUS::IBKVector2QVector(m_planeGeometry.m_vertexes.back());
		m_vertexBufferData.back().m_normal = QVector3D(0,0,1); // not being used
		m_colorBufferData.resize(1);
		m_colorBufferData.back() = QColor(255,0,0); // line is drawn in red
	}

	// now also add a vertex for the current coordinate system's position
	m_vertexBufferData.resize(m_vertexBufferData.size()+1);
	m_vertexBufferData.back().m_coords = m_coordSystemObject->m_transform.translation();
	m_vertexBufferData.back().m_normal = QVector3D(0,0,1); // not being used
	m_colorBufferData.resize(m_colorBufferData.size()+1);
	m_colorBufferData.back() = QColor(255,0,0); // line is drawn in red

	// transfer data stored in m_vertexBufferData
	m_vertexBufferObject.bind();
	m_vertexBufferObject.allocate(m_vertexBufferData.data(), m_vertexBufferData.size()*sizeof(Vertex));
	m_vertexBufferObject.release();

	if (!m_indexBufferData.empty()) {
		m_indexBufferObject.bind();
		m_indexBufferObject.allocate(m_indexBufferData.data(), m_indexBufferData.size()*sizeof(GL_UNSIGNED_SHORT));
		m_indexBufferObject.release();
	}

	m_colorBufferObject.bind();
	m_colorBufferObject.allocate(m_colorBufferData.data(), m_colorBufferData.size()*sizeof(ColorRGBA) );
	m_colorBufferObject.release();
}


void NewPolygonObject::render() {
	// bind all buffers ("position", "normal" and "color" arrays)
	m_vao.bind();
	// now draw the geometry - first the polygon (if any)
	if (!m_indexBufferData.empty())
		glDrawElements(GL_TRIANGLE_STRIP, m_indexBufferData.size(), GL_UNSIGNED_SHORT, nullptr);
	if (m_vertexBufferData.size() > 1)
		// then the line consisting of the last two vertexes
		glDrawArrays(GL_LINES, m_firstLineVertex, 2);
	// release buffers again
	m_vao.release();
}

} // namespace Vic3D
