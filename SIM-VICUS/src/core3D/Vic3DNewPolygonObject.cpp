#include "Vic3DNewPolygonObject.h"

#include <QVector3D>
#include <QOpenGLShaderProgram>
#include <QElapsedTimer>

#include <VICUS_Project.h>
#include <VICUS_Conversions.h>

#include <IBKMK_3DCalculations.h>

#include "SVProjectHandler.h"
#include "SVSettings.h"
#include "SVViewStateHandler.h"
#include "SVPropVertexListWidget.h"
#include "Vic3DGeometryHelpers.h"
#include "Vic3DCoordinateSystemObject.h"
#include "Vic3DShaderProgram.h"

namespace Vic3D {

NewGeometryObject::NewGeometryObject() :
	m_vertexBufferObject(QOpenGLBuffer::VertexBuffer), // VertexBuffer is the default, so default constructor would have been enough
	m_indexBufferObject(QOpenGLBuffer::IndexBuffer) // make this an Index Buffer
{
	// make us known to the world
	SVViewStateHandler::instance().m_newGeometryObject = this;
}


void NewGeometryObject::create(ShaderProgram * shaderProgram) {
	m_shaderProgram = shaderProgram;

	// *** create buffers on GPU memory ***

	// create a new buffer for the vertices and colors, separate buffers because we will modify colors way more often than geometry
	m_vertexBufferObject.create();
	m_vertexBufferObject.setUsagePattern(QOpenGLBuffer::StaticDraw); // usage pattern will be used when tranferring data to GPU

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

	m_vertexBufferObject.bind(); // this registers this buffer data object in the currently bound vao; in subsequent
				  // calls to shaderProgramm->setAttributeBuffer() the buffer object is associated with the
				  // respective attribute array that's fed into the shader. When the vao is later bound before
				  // rendering, this association is remembered so that the vertex fetch stage pulls data from
				  // this vbo

	// coordinates
	m_shaderProgram->shaderProgram()->enableAttributeArray(VERTEX_ARRAY_INDEX);
	m_shaderProgram->shaderProgram()->setAttributeBuffer(VERTEX_ARRAY_INDEX, GL_FLOAT, 0, 3 /* vec3 */, sizeof(VertexC));

	// Release (unbind) all

	// Mind: you can release the buffer data objects (vbo) before or after releasing vao. It does not
	//       matter, because the buffers are associated already with the attribute arrays.
	//       However, YOU MUST NOT release the index buffer (ebo) before releasing the vao, since this would remove
	//       the index buffer association with the vao and when binding the vao before rendering, the element buffer
	//       would not be known and a call to glDrawElements() crashes!
	m_vao.release();

	m_vertexBufferObject.release();
	m_indexBufferObject.release();

	m_planeGeometry = VICUS::PlaneGeometry(VICUS::PlaneGeometry::T_Polygon);
}


void NewGeometryObject::destroy() {
	m_vao.destroy();
	m_vertexBufferObject.destroy();
	m_indexBufferObject.destroy();
}


void NewGeometryObject::appendVertex(const IBKMK::Vector3D & p) {
	switch (m_newGeometryMode) {
		case NGM_Polygon :
		case NGM_ZoneFloor :
			// if we have already a valid plane (i.e. normal vector not 0,0,0), then check if point is in plane
			if (m_planeGeometry.normal() != IBKMK::Vector3D(0,0,0)) {
				IBKMK::Vector3D projected;
				IBKMK::pointProjectedOnPlane(m_planeGeometry.vertexes()[0], m_planeGeometry.normal(), p, projected);
				m_vertexList.push_back(projected);
			}
			else
				m_vertexList.push_back(p);
			m_planeGeometry.setVertexes(m_vertexList);
			// also tell the vertex list widget about our new point
			SVViewStateHandler::instance().m_propVertexListWidget->addVertex(p);
		break;
	}
	// finally update draw buffers
	updateBuffers(false);
}


void NewGeometryObject::appendVertexOffset(const IBKMK::Vector3D & offset) {
	if (m_vertexList.empty())
		appendVertex(offset);
	else
		appendVertex(m_vertexList.back() + offset);
}


void NewGeometryObject::removeVertex(unsigned int idx) {
	Q_ASSERT(idx < m_vertexList.size());
	m_vertexList.erase(m_vertexList.begin()+idx);
	switch (m_newGeometryMode) {
		case NGM_Polygon :
			m_planeGeometry.setVertexes(m_vertexList);
			SVViewStateHandler::instance().m_propVertexListWidget->removeVertex(idx);
		break;
	}
	updateBuffers(false);
}


void NewGeometryObject::removeLastVertex() {
	Q_ASSERT(!m_vertexList.empty());
	removeVertex(m_vertexList.size()-1);
}


void NewGeometryObject::clear() {
	m_planeGeometry.setVertexes(std::vector<IBKMK::Vector3D>());
	m_vertexList.clear();
	updateBuffers(false);
}


bool NewGeometryObject::canComplete() const {
	switch (m_newGeometryMode) {
		case NGM_Polygon :
			return m_planeGeometry.isValid();
	}
	return false;
}


bool NewGeometryObject::hasData() const {
	switch (m_newGeometryMode) {
		case NGM_Polygon :
			return m_planeGeometry.isValid();
	}
	return false;
}


void NewGeometryObject::finish() {
	switch (m_newGeometryMode) {
		case NGM_Polygon :
			if (m_planeGeometry.isValid()) {
				// tell property widget to modify the project with our data
				SVViewStateHandler::instance().m_propVertexListWidget->on_pushButtonFinish_clicked();
			}
			return;
	}
}


void NewGeometryObject::updateLocalCoordinateSystemPosition(const QVector3D & p) {
	QVector3D newPoint = p;

	switch (m_newGeometryMode) {
		case NGM_Polygon :
		case NGM_ZoneFloor :
			// if we have already a valid plane (i.e. normal vector not 0,0,0),
			// then check if point is in plane
			if (m_planeGeometry.normal() != IBKMK::Vector3D(0,0,0)) {
				IBKMK::Vector3D p2(VICUS::QVector2IBKVector(p));
				IBKMK::Vector3D projected;
				IBKMK::pointProjectedOnPlane(m_planeGeometry.vertexes()[0], m_planeGeometry.normal(), p2, projected);
				newPoint = VICUS::IBKVector2QVector(projected);
			}
		break;
	}

	// any change to the previously stored point?
	if (m_localCoordinateSystemPosition == newPoint)
		return;

	// store new position
	m_localCoordinateSystemPosition = newPoint;

	// update buffer (but only that portion that depends on the local coordinate system's location)
	updateBuffers(true);
}


void NewGeometryObject::updateBuffers(bool onlyLocalCSMoved) {
	// create geometry

	// memory layout:
	//   with valid polygon:          vertexBuffer = |polygon_vertexes|coordinate system vertex|
	//   without valid polygon:       vertexBuffer = |last_polygon_vertex|coordinate system vertex|

	// index buffer is only filled if valid polygon exists

	// first copy polygon from PlaneGeometry, if at least 3 vertexes are inserted
	// then add vertex to last

	/* Vertex buffer layout:
		vertexes plane geometry (only for valid polygon)
		vertexes polyline (m_vertexList)
		vertexe coordinate system
	*/

	m_vertexBufferData.clear();
	m_indexBufferData.clear();

	// no vertexes, nothing to draw - we need at least one vertex in the geometry, so that we
	// can draw a line from the last vertex to the current coordinate system's location
	if (m_planeGeometry.vertexes().empty())
		return;

	unsigned int currentVertexIndex = 0;
	unsigned int currentElementIndex = 0;
	if (m_planeGeometry.isValid()) {
		addPlane(m_planeGeometry, currentVertexIndex, currentElementIndex,
				 m_vertexBufferData, m_indexBufferData);
		m_vertexBufferData.push_back( m_vertexBufferData[0] );
	}

	// put all the vertexes of the current polyline into buffer
	// first reserve memory
	m_vertexBufferData.reserve(m_vertexList.size()+ m_vertexBufferData.size()+6);
	for (const IBKMK::Vector3D & v : m_vertexList)
		m_vertexBufferData.push_back( VertexC(VICUS::IBKVector2QVector(v)) );

	m_vertexBufferData.push_back( m_vertexBufferData[0] );

	// now also add a vertex for the current coordinate system's position
	m_vertexBufferData.push_back( VertexC(m_localCoordinateSystemPosition ) );

	// and also the last point of the polygon in order to draw its line blue from local coordinate system
	m_vertexBufferData.push_back( VertexC(VICUS::IBKVector2QVector(m_vertexList.back())) );

	// transfer data stored in m_vertexBufferData
	m_vertexBufferObject.bind();
	m_vertexBufferObject.allocate(m_vertexBufferData.data(), m_vertexBufferData.size()*sizeof(VertexC));
	m_vertexBufferObject.release();

	if (!m_indexBufferData.empty()) {
		m_indexBufferObject.bind();
		m_indexBufferObject.allocate(m_indexBufferData.data(), m_indexBufferData.size()*sizeof(GLuint));
		m_indexBufferObject.release();
	}

}


void NewGeometryObject::renderOpqaue() {
	if (m_vertexBufferData.empty())
		return;

	// bind all buffers
	m_vao.bind();
	// set transformation matrix - unity matrix, since we draw with world coordinates
	m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[1], QMatrix4x4());

	// draw the polygon line first
	if (m_vertexBufferData.size() > 1) {

		QColor lineCol;
		if (m_planeGeometry.isValid()) {
			if ( SVSettings::instance().m_theme == SVSettings::TT_Dark )
				lineCol = QColor("#2ed655");
			else
				lineCol = QColor("#00ff48");
		}
		else {
			if ( SVSettings::instance().m_theme == SVSettings::TT_Dark )
				lineCol = QColor("#c31818");
			else
				lineCol = QColor("#ed1f1f");
		}

		m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[2], lineCol);
		// then the line consisting of the last two vertexes

		// line drawing starts from vertex 0 if polygon is invalid, or
		// from first vertex after polygon-vertexes
		size_t offset = 0;
		if (m_planeGeometry.isValid())
			offset = m_planeGeometry.vertexes().size() + 1; // +1, because we have added the first polygon again at the end
		// paint a line for each of the vertexes in the list and one extra, for the trailing polygon line
		for (size_t i = 1; i < m_vertexList.size()+1; ++i, ++offset) {
			glDrawArrays(GL_LINES, offset, 2);
		}
		// set wireframe color (TODO : make this theme-dependent?)
		if ( SVSettings::instance().m_theme == SVSettings::TT_Dark )
			lineCol = QColor("#32c5ea");
		else
			lineCol = QColor("#106d90");


		m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[2], lineCol);
		glDrawArrays(GL_LINES, offset, 2);
		glDrawArrays(GL_LINES, ++offset, 2);
//		glDrawArrays(GL_LINES, ++offset, 2);
	}

	// now draw the geometry
	if (!m_indexBufferData.empty()) {
		glDisable(GL_CULL_FACE);
#if 1
		// set wireframe color (TODO : make this theme-dependent?)
		QColor wireFrameCol;
		if ( SVSettings::instance().m_theme == SVSettings::TT_Dark )
			wireFrameCol = QColor(255,255,255,255);
		else
			wireFrameCol = QColor(120,120,120,255);

		m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[2], wireFrameCol);
		// select wire frame drawing
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		// now draw the geometry
		glDrawElements(GL_TRIANGLES, m_indexBufferData.size(), GL_UNSIGNED_INT, nullptr);
		// switch back to fill mode
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
	}

	// release buffers again
	m_vao.release();
}


void NewGeometryObject::renderTransparent() {
	// we expect:
	//   glDepthMask(GL_FALSE);
	//   shader program has already transform uniform set
	//   glDisable(GL_CULL_FACE);

	if (!m_indexBufferData.empty()) {
		// bind all buffers
		m_vao.bind();
		// set transformation matrix - unity matrix, since we draw with world coordinates
		m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[1], QMatrix4x4());

		// now draw the geometry

		// disable updating of z-buffer
		glDepthMask(GL_FALSE);
		// set selected plane color (QColor is passed as vec4, so no conversion is needed, here).
		QColor planeCol = QColor(255,0,128,64);
		m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[2], planeCol);
		// put OpenGL in offset mode
		glEnable(GL_POLYGON_OFFSET_FILL);
		// offset plane geometry a bit so that the plane is drawn behind the wireframe
		glPolygonOffset(0.1f, 2.0f);
		// now draw the geometry
		glDrawElements(GL_TRIANGLES, m_indexBufferData.size(), GL_UNSIGNED_INT, nullptr);
		// turn off line offset mode
		glDisable(GL_POLYGON_OFFSET_FILL);

		glEnable(GL_CULL_FACE);

		// release buffers again
		m_vao.release();
	}
}

} // namespace Vic3D
